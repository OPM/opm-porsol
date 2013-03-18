/*
  Copyright 2010 SINTEF ICT, Applied Mathematics.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPM_TPFACOMPRESSIBLEASSEMBLER_HEADER_INCLUDED
#define OPM_TPFACOMPRESSIBLEASSEMBLER_HEADER_INCLUDED


#include <opm/core/pressure/tpfa/cfs_tpfa.h>
#include <opm/core/pressure/tpfa/trans_tpfa.h>
#include <opm/core/linalg/sparse_sys.h>
#include <opm/core/pressure/flow_bc.h>
#include <opm/core/pressure/legacy_well.h>
#include <opm/core/pressure/tpfa/compr_quant.h>
#include <dune/grid/common/GridAdapter.hpp>
#include <stdexcept>



/// @brief
///     Encapsulates the cfs_tpfa (= compressible flow solver
///     two-point flux approximation) solver modules.
class TpfaCompressibleAssembler
{
public:
    /// @brief
    ///     Default constructor, does nothing.
    TpfaCompressibleAssembler()
        :  state_(Uninitialized), data_(0), bc_(0)
    {
        wells_.number_of_wells = 0;
    }




    /// @brief
    ///     Destructor.
    ~TpfaCompressibleAssembler()
    {
        flow_conditions_destroy(bc_);
        cfs_tpfa_destroy(data_);
    }




    /// @brief
    ///     Initialize the solver's structures for a given grid, for well setup also call initWells().
    /// @tparam Grid This must conform to the SimpleGrid concept.
    /// @tparam Wells This must conform to the SimpleWells concept.
    /// @param grid The grid object.
    /// @param wells Well specifications.
    /// @param perm Permeability. It should contain dim*dim entries (a full tensor) for each cell.
    /// @param perm Porosity by cell.
    /// @param gravity Array containing gravity acceleration vector. It should contain dim entries.
    template <class Grid, class Wells>
    void init(const Grid& grid, const Wells& wells, const double* perm, const double* porosity,
              const typename Grid::Vector& gravity)
    {
        initWells(wells);
        init(grid, perm, porosity, gravity);
    }

    /// @brief
    ///     Initialize the solver's structures for a given grid, for well setup also call initWells().
    /// @tparam Grid This must conform to the SimpleGrid concept.
    /// @param grid The grid object.
    /// @param perm Permeability. It should contain dim*dim entries (a full tensor) for each cell.
    /// @param gravity Array containing gravity acceleration vector. It should contain dim entries.
    template <class Grid>
    void init(const Grid& grid, const double* perm, const double* porosity, const typename Grid::Vector& gravity)
    {
        // Build C grid structure.
        grid_.init(grid);

        // Initialize data.
        int num_phases = 3;
        well_t* w = 0;
        if (wells_.number_of_wells != 0) {
            w = &wells_;
        }
        data_ = cfs_tpfa_construct(grid_.c_grid(), w, num_phases);
        if (!data_) {
            throw std::runtime_error("Failed to initialize cfs_tpfa solver.");
        }

        // Compute half-transmissibilities
        int num_cells = grid.numCells();
        int ngconn  = grid_.c_grid()->cell_facepos[num_cells];
        ncf_.resize(num_cells);
        for (int cell = 0; cell < num_cells; ++cell) {
            int num_local_faces = grid.numCellFaces(cell);
            ncf_[cell] = num_local_faces;
        }
        htrans_.resize(ngconn);
        tpfa_htrans_compute(grid_.c_grid(), perm, &htrans_[0]);

        // Compute transmissibilities.
        trans_.resize(grid_.numFaces());
        tpfa_trans_compute(grid_.c_grid(), &htrans_[0], &trans_[0]);

        // Compute pore volumes.
        porevol_.resize(num_cells);
        for (int i = 0; i < num_cells; ++i) {
            porevol_[i] = porosity[i]*grid.cellVolume(i);
        }

        // Set gravity.
        if (Grid::dimension != 3) {
            throw std::logic_error("Only 3 dimensions supported currently.");
        }
        std::copy(gravity.begin(), gravity.end(), gravity_);

        state_ = Initialized;
    }



    /// Boundary condition types.
    enum FlowBCTypes { FBC_UNSET    = BC_NOFLOW      ,
                       FBC_PRESSURE = BC_PRESSURE    ,
                       FBC_FLUX     = BC_FLUX_TOTVOL };

    /// @brief
    /// Assemble the sparse system.
    /// You must call init() prior to calling assemble().
    /// @param sources Source terms, one per cell. Positive numbers
    /// are sources, negative are sinks.
    /// @param total_mobilities Scalar total mobilities, one per cell.
    /// @param omegas Gravity term, one per cell. In a multi-phase
    /// flow setting this is equal to
    /// \f[ \omega = \sum_{p} \frac{\lambda_p}{\lambda_t} \rho_p \f]
    /// where \f$\lambda_p\f$ is a phase mobility, \f$\rho_p\f$ is a
    /// phase density and \f$\lambda_t\f$ is the total mobility.
    void assemble(const double* sources,
                  const FlowBCTypes* bctypes,
                  const double* bcvalues,
                  const double dt,
                  const double* totcompr,
                  const double* voldiscr,
                  const double* cellA,  // num phases^2 * num cells, fortran ordering!
                  const double* faceA,  // num phases^2 * num faces, fortran ordering!
                  const double* wellperfA,
                  const double* phasemobf,
                  const double* phasemobwellperf,
                  const double* cell_pressure,
                  const double* gravcapf,
                  const double* wellperf_gpot,
                  const double* surf_dens)
    {
        if (state_ == Uninitialized) {
            throw std::runtime_error("Error in TpfaCompressibleAssembler::assemble(): You must call init() prior to calling assemble().");
        }
        UnstructuredGrid* g = grid_.c_grid();

        // Boundary conditions.
        gather_boundary_conditions(bctypes, bcvalues);

        // Source terms from user.
        double* src = const_cast<double*>(sources); // Ugly? Yes. Safe? I think so.

        // Wells.
        well_t* wells = NULL;
        well_control_t* wctrl = NULL;
        struct completion_data* wcompl = NULL;
        if (wells_.number_of_wells != 0) {
            wells = &wells_;
            wctrl = &wctrl_;
            wcompl = &wcompl_;
            // The next objects already have the correct sizes.
            std::copy(wellperf_gpot, wellperf_gpot + well_gpot_storage_.size(), well_gpot_storage_.begin());
            std::copy(wellperfA, wellperfA + well_A_storage_.size(), well_A_storage_.begin());
            std::copy(phasemobwellperf, phasemobwellperf + well_phasemob_storage_.size(), well_phasemob_storage_.begin());
        }

        // Assemble the embedded linear system.
        compr_quantities cq = { 3                               ,
                                const_cast<double *>(totcompr ) ,
                                const_cast<double *>(voldiscr ) ,
                                const_cast<double *>(cellA    ) ,
                                const_cast<double *>(faceA    ) ,
                                const_cast<double *>(phasemobf) };

        // Call the assembly routine. After this, linearSystem() may be called.
        cfs_tpfa_assemble(g, dt, wells, bc_, src,
                          &cq, &trans_[0], gravcapf,
                          wctrl, wcompl,
                          cell_pressure, &porevol_[0],
                          data_);
        phasemobf_.assign(phasemobf, phasemobf + grid_.numFaces()*3);
        gravcapf_.assign(gravcapf, gravcapf + grid_.numFaces()*3);
        state_ = Assembled;
    }




    /// Encapsulate a sparse linear system in CSR format.
    struct LinearSystem
    {
        int n;
        int nnz;
        int* ia;
        int* ja;
        double* sa;
        double* b;
        double* x;
    };

    /// @brief
    /// Access the linear system assembled.
    /// You must call assemble() prior to calling linearSystem().
    /// @param[out] s The linear system encapsulation to modify.
    /// After this call, s will point to linear system structures
    /// that are owned and allocated internally.
    void linearSystem(LinearSystem& s)

    {
        if (state_ != Assembled) {
            throw std::runtime_error("Error in TpfaCompressibleAssembler::linearSystem(): "
                                     "You must call assemble() prior to calling linearSystem().");
        }
        s.n = data_->A->m;
        s.nnz = data_->A->nnz;
        s.ia = data_->A->ia;
        s.ja = data_->A->ja;
        s.sa = data_->A->sa;
        s.b = data_->b;
        s.x = data_->x;
    }




    /// @brief
    /// Compute cell pressures and face fluxes.
    /// You must call assemble() (and solve the linear system accessed
    /// by calling linearSystem()) prior to calling
    /// computePressuresAndFluxes().
    /// @param[out] cell_pressures Cell pressure values.
    /// @param[out] face_areas Face flux values.
    void computePressuresAndFluxes(std::vector<double>& cell_pressures,
                                   std::vector<double>& face_pressures,
                                   std::vector<double>& face_fluxes,
                                   std::vector<double>& well_pressures,
                                   std::vector<double>& well_fluxes)
    {
        if (state_ != Assembled) {
            throw std::runtime_error("Error in TpfaCompressibleAssembler::computePressuresAndFluxes(): "
                                     "You must call assemble() (and solve the linear system) "
                                     "prior to calling computePressuresAndFluxes().");
        }
        int num_cells = grid_.c_grid()->number_of_cells;
        int num_faces = grid_.c_grid()->number_of_faces;
        cell_pressures.clear();
        cell_pressures.resize(num_cells, 0.0);
        face_pressures.clear();
        face_pressures.resize(num_faces, 0.0);
        face_fluxes.clear();
        face_fluxes.resize(num_faces, 0.0);

        int np = 3; // Number of phases.

        // Wells.
        well_t* wells = NULL;
        struct completion_data* wcompl = NULL;
        double* wpress = 0;
        double* wflux = 0;
        if (wells_.number_of_wells != 0) {
            wells = &wells_;
            wcompl = &wcompl_;
            well_pressures.resize(wells_.number_of_wells);
            well_fluxes.resize(well_cells_storage_.size());
            wpress = &well_pressures[0];
            wflux = &well_fluxes[0];
        }

        cfs_tpfa_press_flux(grid_.c_grid(),
                            bc_, wells,
                            np, &trans_[0], &phasemobf_[0], &gravcapf_[0],
                            wcompl,
                            data_, &cell_pressures[0], &face_fluxes[0],
                            wpress, wflux);
        cfs_tpfa_fpress(grid_.c_grid(), bc_, np, &htrans_[0],
                        &phasemobf_[0], &gravcapf_[0], data_, &cell_pressures[0],
                        &face_fluxes[0], &face_pressures[0]);
    }




    /// @brief
    ///     Explicit IMPES time step limit.
    double explicitTimestepLimit(const double* faceA,  // num phases^2 * num faces, fortran ordering!
                                 const double* phasemobf,
                                 const double* phasemobf_deriv,
                                 const double* surf_dens)
    {
        compr_quantities cq = { 3, // nphases
                                0, // totcompr
                                0, // voldiscr
                                0, // Ac
                                const_cast<double *>(faceA)    ,
                                const_cast<double *>(phasemobf) };
        return cfs_tpfa_impes_maxtime(grid_.c_grid(), &cq, &trans_[0], &porevol_[0], data_,
                                      phasemobf_deriv, surf_dens, gravity_);
    }



    /// @brief
    ///     Explicit IMPES transport.
    void explicitTransport(const double dt,
                           double* cell_surfvols)
    {
        int np = 3; // Number of phases.

        well_t* wells = NULL;
        if (wells_.number_of_wells != 0) {
            wells = &wells_;
        }
        cfs_tpfa_expl_mass_transport(grid_.c_grid(), wells, &wcompl_,
                                     np, dt, &porevol_[0],
                                     data_, cell_surfvols);
    }




    /// @brief
    /// Compute cell fluxes from face fluxes.
    /// You must call assemble() (and solve the linear system accessed
    /// by calling linearSystem()) prior to calling
    /// faceFluxToCellFlux().
    /// @param face_fluxes 
    /// @param face_areas Face flux values (usually output from computePressuresAndFluxes()).
    /// @param[out] cell_fluxes Cell-wise flux values.
    /// They are given in cell order, and for each cell there is
    /// one value for each adjacent face (in the same order as the
    /// cell-face topology of the grid). Positive values represent
    /// fluxes out of the cell.
    void faceFluxToCellFlux(const std::vector<double>& face_fluxes,
                            std::vector<double>& cell_fluxes)
    {
        if (state_ != Assembled) {
            throw std::runtime_error("Error in TpfaCompressibleAssembler::faceFluxToCellFlux(): "
                                     "You must call assemble() (and solve the linear system) "
                                     "prior to calling faceFluxToCellFlux().");
        }
        const UnstructuredGrid& g = *(grid_.c_grid());
        int num_cells = g.number_of_cells;
        cell_fluxes.resize(g.cell_facepos[num_cells]);
        for (int cell = 0; cell < num_cells; ++cell) {
            for (int hface = g.cell_facepos[cell]; hface < g.cell_facepos[cell + 1]; ++hface) {
                int face = g.cell_faces[hface];
                bool pos = (g.face_cells[2*face] == cell);
                cell_fluxes[hface] = pos ? face_fluxes[face] : -face_fluxes[face];
            }
        }
    }




    /// @brief
    /// Access the number of connections (faces) per cell. Deprecated, will be removed.
    const std::vector<int>& numCellFaces() const
    {
        return ncf_;
    }


    const std::vector<double>& faceTransmissibilities() const
    {
        return trans_;
    }

private:
    // Disabling copy and assigment for now.
    TpfaCompressibleAssembler(const TpfaCompressibleAssembler&);
    TpfaCompressibleAssembler& operator=(const TpfaCompressibleAssembler&);

    enum State { Uninitialized, Initialized, Assembled };
    State state_;

    // Solver data.
    cfs_tpfa_data* data_;
    // Grid.
    GridAdapter grid_;
    // Number of faces per cell.
    std::vector<int> ncf_;
    // Transmissibility storage.
    std::vector<double> htrans_;
    std::vector<double> trans_;
    // Pore volumes.
    std::vector<double> porevol_;
    // Phase mobilities per face.
    std::vector<double> phasemobf_;
    // Gravity and capillary contributions (per face).
    std::vector<double> gravcapf_;
    // Gravity
    double gravity_[3];

    // Boundary conditions.
    FlowBoundaryConditions *bc_;

    // Well data
    well_t wells_;
    std::vector<int> well_connpos_storage_;
    std::vector<int> well_cells_storage_;
    well_control_t wctrl_;
    std::vector<well_type> wctrl_type_storage_;
    std::vector<well_control> wctrl_ctrl_storage_;
    std::vector<double> wctrl_target_storage_;
    struct completion_data wcompl_;
    std::vector<double> well_prodind_storage_;
    std::vector<double> well_gpot_storage_;
    std::vector<double> well_A_storage_;
    std::vector<double> well_phasemob_storage_;


    /// @brief
    ///     Initialize wells in solver structure.
    /// @tparam Wells
    ///     This must conform to the SimpleWells concept.
    /// @param w
    ///     The well object.
    template <class Wells>
    void initWells(const Wells& w)
    {
        int num_wells = w.numWells();
        if (num_wells == 0) {
            wells_.number_of_wells = 0;
            return;
        }
        wctrl_type_storage_.resize(num_wells);
        wctrl_ctrl_storage_.resize(num_wells);
        wctrl_target_storage_.resize(num_wells);
        for (int i = 0; i < num_wells; ++i) {
            wctrl_type_storage_[i] = (w.type(i) == Wells::Injector) ? INJECTOR : PRODUCER;
            wctrl_ctrl_storage_[i] = (w.control(i) == Wells::Rate) ? RATE : BHP;
            wctrl_target_storage_[i] = w.target(i);
            int num_perf = w.numPerforations(i);
            well_connpos_storage_.push_back(well_cells_storage_.size());
            for (int j = 0; j < num_perf; ++j) {
                well_cells_storage_.push_back(w.wellCell(i, j));
                well_prodind_storage_.push_back(w.wellIndex(i, j));
            }
        }
        well_connpos_storage_.push_back(well_cells_storage_.size());
        int tot_num_perf = well_prodind_storage_.size();
        well_gpot_storage_.resize(tot_num_perf*3);
        well_A_storage_.resize(3*3*tot_num_perf);
        well_phasemob_storage_.resize(3*tot_num_perf);
        // Setup 'wells_'
        wells_.number_of_wells = num_wells;
        wells_.well_connpos = &well_connpos_storage_[0];
        wells_.well_cells = &well_cells_storage_[0];
        // Setup 'wctrl_'
        wctrl_.type = &wctrl_type_storage_[0];
        wctrl_.ctrl = &wctrl_ctrl_storage_[0];
        wctrl_.target = &wctrl_target_storage_[0];
        // Setup 'wcompl_'
        wcompl_.WI = &well_prodind_storage_[0];
        wcompl_.gpot = &well_gpot_storage_[0];
        wcompl_.A = &well_A_storage_[0];
        wcompl_.phasemob = &well_phasemob_storage_[0];
    }


    void
    gather_boundary_conditions(const FlowBCTypes* bctypes ,
                               const double*      bcvalues)
    {
        if (bc_ == 0) {
            bc_ = flow_conditions_construct(0);
        }
        else {
            flow_conditions_clear(bc_);
        }

        int ok = bc_ != 0;

        for (std::size_t i = 0, nf = grid_.numFaces(); ok && (i < nf); ++i) {
            if (bctypes[ i ] == FBC_PRESSURE) {
                ok = flow_conditions_append(BC_PRESSURE,
                                            static_cast<int>(i),
                                            bcvalues[ i ],
                                            bc_);
            }
            else if (bctypes[ i ] == FBC_FLUX) {
                ok = flow_conditions_append(BC_FLUX_TOTVOL,
                                            static_cast<int>(i),
                                            bcvalues[ i ],
                                            bc_);
            }
        }

        if (! ok) {
            flow_conditions_destroy(bc_);
            bc_ = 0;
        }
    }


}; // class TpfaCompressibleAssembler




#endif // OPM_TPFACOMPRESSIBLEASSEMBLER_HEADER_INCLUDED
