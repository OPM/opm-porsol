//===========================================================================
//
// File: mimetic_solver_test.cpp
//
// Created: Tue Jul  7 15:35:21 2009
//
// Author(s): B�rd Skaflestad     <bard.skaflestad@sintef.no>
//            Atgeirr F Rasmussen <atgeirr@sintef.no>
//
// $Date: 2009-09-01 20:55:38 +0200 (Tue, 01 Sep 2009) $
//
// $Revision: 441 $
//
//===========================================================================

/*
  Copyright 2009, 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2009, 2010 Statoil ASA.

  This file is part of The Open Reservoir Simulator Project (OpenRS).

  OpenRS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OpenRS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenRS.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"

#include <algorithm>
#include <iostream>
#include <iomanip>


#include <array>

#include <dune/common/version.hh>
#if DUNE_VERSION_NEWER(DUNE_COMMON, 2, 3)
#include <dune/common/parallel/mpihelper.hh>
#else
#include <dune/common/mpihelper.hh>
#endif

#include <opm/core/utility/Units.hpp>

#include <dune/grid/yaspgrid.hh>
#include <dune/grid/CpGrid.hpp>
#include <opm/core/io/eclipse/EclipseGridInspector.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>

#include <opm/porsol/common/fortran.hpp>
#include <opm/porsol/common/blas_lapack.hpp>
#include <opm/porsol/common/Matrix.hpp>
#include <opm/porsol/common/GridInterfaceEuler.hpp>
#include <opm/porsol/common/ReservoirPropertyCapillaryAnisotropicRelperm.hpp>
#include <opm/porsol/common/BoundaryConditions.hpp>

#include <opm/porsol/mimetic/MimeticIPAnisoRelpermEvaluator.hpp>
#include <opm/porsol/mimetic/IncompFlowSolverHybrid.hpp>
#include <opm/core/utility/parameters/ParameterGroup.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>


template <int dim, class Interface>
void test_evaluator(const Interface& g)
{
    typedef typename Interface::CellIterator CI;
    typedef typename CI       ::FaceIterator FI;
    typedef typename CI       ::Scalar       Scalar;

    typedef Opm::SharedFortranMatrix FMat;

    std::cout << "Called test_evaluator()" << std::endl;

    std::vector<int> numf; numf.reserve(g.numberOfCells());
    int max_nf = -1;
    for (CI c = g.cellbegin(); c != g.cellend(); ++c) {
        numf.push_back(0);
        int& nf = numf.back();

        for (FI f = c->facebegin(); f != c->faceend(); ++f)
            ++nf;

        max_nf = std::max(max_nf, nf);
    }

    typedef int DummyClass;
    Opm::MimeticIPAnisoRelpermEvaluator<Interface, DummyClass> ip(max_nf);

    // Set dummy permeability K=diag(10,1,...,1,0.1).
    std::vector<Scalar> perm(dim * dim, Scalar(0.0));
    Opm::SharedCMatrix K(dim, dim, &perm[0]);
    for (int i = 0; i < dim; ++i)
        K(i,i) = 1.0;
    K(0    ,0    ) *= 10.0;
    K(dim-1,dim-1) /= 10.0;

    // Storage for inverse ip.
    std::vector<Scalar> ip_store(max_nf * max_nf, Scalar(0.0));

    // Loop grid whilst building (and outputing) the inverse IP matrix.
    int count = 0;
    for (CI c = g.cellbegin(); c != g.cellend(); ++c, ++count) {
        FMat Binv(numf[count], numf[count], &ip_store[0]);

        ip.evaluate(c, K, Binv);

        std::cout << count << " -> Binv = [\n" << Binv << "]\n";
    }
}


void build_grid(Opm::DeckConstPtr deck,
                const double z_tol, Dune::CpGrid& grid,
                std::array<int,3>& cartDims)
{
    Opm::EclipseGridInspector insp(deck);

    grdecl g;
    cartDims[0] = g.dims[0] = insp.gridSize()[0];
    cartDims[1] = g.dims[1] = insp.gridSize()[1];
    cartDims[2] = g.dims[2] = insp.gridSize()[2];

    g.coord = &deck->getKeyword("COORD").getSIDoubleData()[0];
    g.zcorn = &deck->getKeyword("ZCORN").getSIDoubleData()[0];

    if (deck->hasKeyword("ACTNUM")) {
        g.actnum = &deck->getKeyword("ACTNUM").getIntData()[0];
        grid.processEclipseFormat(g, z_tol, false, false);
    } else {
        std::vector<int> dflt_actnum(g.dims[0] * g.dims[1] * g.dims[2], 1);

        g.actnum = &dflt_actnum[0];
        grid.processEclipseFormat(g, z_tol, false, false);
    }
}


template<int dim, class RI>
void assign_permeability(RI& r, int nc, double k)
{
    typedef typename RI::SharedPermTensor Tensor;
    for (int c = 0; c < nc; ++c) {
        Tensor K = r.permeabilityModifiable(c);
        for (int i = 0; i < dim; ++i) {
            K(i,i) = k;
        }
    }
}


template<int dim, class GI, class RI>
void test_flowsolver(const GI& g, const RI& r)
{
    typedef typename GI::CellIterator                              CI;
    typedef Opm::BasicBoundaryConditions<true, false>                  FBC;
    typedef Opm::IncompFlowSolverHybrid<GI, RI, FBC,
                                         Opm::MimeticIPAnisoRelpermEvaluator> FlowSolver;

    FlowSolver solver;

    typedef Opm::FlowBC BC;
    FBC flow_bc(7);
    //flow_bc.flowCond(1) = BC(BC::Dirichlet, 1.0*Opm::unit::barsa);
    //flow_bc.flowCond(2) = BC(BC::Dirichlet, 0.0*Opm::unit::barsa);
    flow_bc.flowCond(5) = BC(BC::Dirichlet, 100.0*Opm::unit::barsa);

    typename CI::Vector gravity;
    gravity[0] = gravity[1] = 0.0;
    gravity[2] = Opm::unit::gravity;

    solver.init(g, r, gravity, flow_bc);
    // solver.printStats(std::cout);

    //solver.assembleStatic(g, r);
    //solver.printIP(std::cout);

    std::vector<double> src(g.numberOfCells(), 0.0);
    std::vector<double> sat(g.numberOfCells(), 0.0);
#if 0
    if (g.numberOfCells() > 1) {
        src[0]     = 1.0;
        src.back() = -1.0;
    }
#endif

    solver.solve(r, sat, flow_bc, src);

#if 0
    solver.printSystem("system");
    typedef typename FlowSolver::SolutionType FlowSolution;
    FlowSolution soln = solver.getSolution();
    std::cout << "Cell Pressure:\n" << std::scientific << std::setprecision(15);
    for (CI c = g.cellbegin(); c != g.cellend(); ++c) {
        std::cout << '\t' << soln.pressure(c) << '\n';
    }

    std::cout << "Cell (Out) Fluxes:\n";
    std::cout << "flux = [\n";
    for (CI c = g.cellbegin(); c != g.cellend(); ++c) {
        for (FI f = c->facebegin(); f != c->faceend(); ++f) {
            std::cout << soln.outflux(f) << ' ';
        }
        std::cout << "\b\n";
    }
    std::cout << "]\n";
#endif
}


using namespace Opm;

int main(int argc, char** argv)
try
{
    Opm::parameter::ParameterGroup param(argc, argv);
    Dune::MPIHelper::instance(argc,argv);

    // Make a grid
    Dune::CpGrid grid;

    Opm::ParseContext parseContext;
    Opm::ParserPtr parser(new Opm::Parser());
    Opm::DeckConstPtr deck(parser->parseFile(param.get<std::string>("filename") , parseContext));
    double z_tol = param.getDefault<double>("z_tolerance", 0.0);
    std::array<int,3> cartDims;
    build_grid(deck, z_tol, grid, cartDims);

    // Make the grid interface
    Opm::GridInterfaceEuler<Dune::CpGrid> g(grid);

    // Reservoir properties.
    typedef ReservoirPropertyCapillaryAnisotropicRelperm<3> RP;
    RP res_prop;
    res_prop.init(deck, grid.globalCell());

    assign_permeability<3>(res_prop, g.numberOfCells(), 0.1*Opm::unit::darcy);
    test_flowsolver<3>(g, res_prop);

#if 0
    // Make flow equation boundary conditions.
    // Pressure 1.0e5 on the left, 0.0 on the right.
    // Recall that the boundary ids range from 1 to 6 for the cartesian edges,
    // and that boundary id 0 means interiour face/intersection.
    typedef FlowBoundaryCondition BC;
    FlowBoundaryConditions flow_bcond(7);
    flow_bcond[1] = BC(BC::Dirichlet, 1.0e5);
    flow_bcond[2] = BC(BC::Dirichlet, 0.0);

    // Make transport equation boundary conditions.
    // The default one is fine (sat = 1.0 on inflow).
    SaturationBoundaryConditions sat_bcond(7);

    // No injection or production.
    SparseVector<double> injection_rates(g.numberOfCells());

    // Make a solver.
    typedef EulerUpstream<GridInterface,
                          RP,
                          SaturationBoundaryConditions> TransportSolver;
    TransportSolver transport_solver(g, res_prop, sat_bcond, injection_rates);

    // Define a flow field with constant velocity
    Dune::FieldVector<double, 3> vel(0.0);
    vel[0] = 2.0;
    vel[1] = 1.0;
    TestSolution<GridInterface> flow_solution(g, vel);
    // Solve a step.
    double time = 1.0;
    std::vector<double> sat(g.numberOfCells(), 0.0);
    Dune::FieldVector<double, 3> gravity(0.0);
    gravity[2] = -9.81;
    transport_solver.transportSolve(sat, time, gravity, flow_solution);
#endif
}
catch (const std::exception &e) {
    std::cerr << "Program threw an exception: " << e.what() << "\n";
    throw;
}



#if 0
int main (int argc , char **argv) try {
    try {
#if HAVE_MPI
	// initialize MPI
	MPI_Init(&argc,&argv);
	// get own rank
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
#endif
	//check_yasp<3,0>();  // 3D, 1 x 1 x 1 cell
	check_cpgrid<0>();
	check_cpgrid<1>();
	check_cpgrid<2>();

    } catch (Dune::Exception &e) {
	std::cerr << e << std::endl;
	return 1;
    } catch (...) {
	std::cerr << "Generic exception!" << std::endl;
	return 2;
    }

#if HAVE_MPI
    // Terminate MPI
    MPI_Finalize();
#endif

    return 0;
}
catch (const std::exception &e) {
    std::cerr << "Program threw an exception: " << e.what() << "\n";
    throw;
}

#endif
