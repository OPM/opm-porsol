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


#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <algorithm>
#include <iostream>
#include <iomanip>

#include <boost/static_assert.hpp>

#include <dune/common/array.hh>
#include <dune/common/mpihelper.hh>
#include <dune/common/Units.hpp>

#include <dune/porsol/common/SimulatorUtilities.hpp>
#include <dune/grid/io/file/vtk/vtkwriter.hh>

#include <dune/grid/yaspgrid.hh>
#include <dune/grid/CpGrid.hpp>
#include <dune/common/EclipseGridParser.hpp>
#include <dune/common/EclipseGridInspector.hpp>

#include <dune/porsol/common/fortran.hpp>
#include <dune/porsol/common/blas_lapack.hpp>
#include <dune/porsol/common/Matrix.hpp>
#include <dune/porsol/common/GridInterfaceEuler.hpp>
#include <dune/porsol/common/ReservoirPropertyCapillary.hpp>
#include <dune/porsol/common/BoundaryConditions.hpp>

#include <dune/porsol/mimetic/TpfaCompressible.hpp>
#include <dune/common/param/ParameterGroup.hpp>
#include <dune/porsol/common/setupGridAndProps.hpp>


template<int dim, class GI, class RI>
void test_flowsolver(const GI& g, const RI& r, double dt)
{
    typedef typename GI::CellIterator                   CI;
    typedef typename CI::FaceIterator                   FI;
    typedef Dune::BasicBoundaryConditions<true, false>  FBC;
    typedef Dune::TpfaCompressible<GI, RI, FBC> FlowSolver;
    // typedef Dune::TpfaInterface<GI, RI, FBC> FlowSolver;
    // typedef Dune::IfshInterface<GI, RI, FBC> FlowSolver;
    // typedef Dune::IncompFlowSolverHybrid<GI, RI, FBC,
    //                                   Dune::MimeticIPEvaluator> FlowSolver;

    FlowSolver solver;

    typedef Dune::FlowBC BC;
    FBC flow_bc(7);

//     flow_bc.flowCond(5) = BC(BC::Dirichlet, 100.0*Dune::unit::barsa);
//     flow_bc.flowCond(6) = BC(BC::Dirichlet, 0.0*Dune::unit::barsa);

    typename CI::Vector gravity(0.0);
//     gravity[2] = Dune::unit::gravity;

    solver.init(g, r, gravity, flow_bc);

    std::vector<double> src(g.numberOfCells(), 0.0);
    std::vector<double> sat(g.numberOfCells(), 0.0);
    if (g.numberOfCells() > 1) {
        src[0]     = 1.0;
        src.back() = -1.0;
    }
    std::vector<double> cell_pressure(g.numberOfCells(), 0.0);

    solver.solve(r, cell_pressure, sat, flow_bc, src, dt, 1e-8, 3, 1);


    typedef typename FlowSolver::SolutionType FlowSolution;
    FlowSolution soln = solver.getSolution();

    std::vector<typename GI::Vector> cell_velocity;
    estimateCellVelocity(cell_velocity, g, soln);
    // Dune's vtk writer wants multi-component data to be flattened.
    std::vector<double> cell_velocity_flat(&*cell_velocity.front().begin(),
                                           &*cell_velocity.back().end());
    getCellPressure(cell_pressure, g, soln);

    Dune::VTKWriter<typename GI::GridType::LeafGridView> vtkwriter(g.grid().leafView());
    vtkwriter.addCellData(cell_pressure, "pressure");
    vtkwriter.addCellData(cell_velocity_flat, "velocity", dim);
    vtkwriter.write("testsolution-" + boost::lexical_cast<std::string>(0),
                    Dune::VTKOptions::ascii);
}


using namespace Dune;

int main(int argc, char** argv)
{
    Dune::parameter::ParameterGroup param(argc, argv);
    Dune::MPIHelper::instance(argc,argv);

    // Make a grid and props.
    Dune::CpGrid grid;
    ReservoirPropertyCapillary<3> res_prop;
    setupGridAndProps(param, grid, res_prop);

    // Make the grid interface.
    Dune::GridInterfaceEuler<Dune::CpGrid> g(grid);

    double dt = param.getDefault("dt", 1.0);

    // Run test.
    test_flowsolver<3>(g, res_prop, dt);
}

