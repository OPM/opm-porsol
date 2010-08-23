//===========================================================================
//
// File: mimetic_ipeval_test.cpp
//
// Created: Mon Jun 29 11:09:14 2009
//
// Author(s): B�rd Skaflestad     <bard.skaflestad@sintef.no>
//            Atgeirr F Rasmussen <atgeirr@sintef.no>
//
// $Date$
//
// $Revision$
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


#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <algorithm>
#include <iostream>

#include <boost/static_assert.hpp>

#include <dune/common/array.hh>
#include <dune/grid/yaspgrid.hh>
#include <dune/grid/CpGrid.hpp>

#include <dune/solvers/common/fortran.hpp>
#include <dune/solvers/common/blas_lapack.hpp>
#include <dune/solvers/common/Matrix.hpp>
#include <dune/solvers/common/GridInterfaceEuler.hpp>

#include <dune/solvers/mimetic/MimeticIPEvaluator.hpp>
#include <dune/solvers/mimetic/IncompFlowSolverHybrid.hpp>

template <int dim, class Interface>
void test_evaluator(const Interface& g)
{
    typedef typename Interface::CellIterator CI;
    typedef typename CI       ::FaceIterator FI;
    typedef typename CI       ::Scalar       Scalar;

    typedef Dune::SharedFortranMatrix FMat;

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
    Dune::MimeticIPEvaluator<Interface, DummyClass> ip(max_nf);

    // Set dummy permeability K=diag(10,1,...,1,0.1).
    std::vector<Scalar> perm(dim * dim, Scalar(0.0));
    Dune::SharedCMatrix K(dim, dim, &perm[0]);
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


template <int dim, int refinement>
void check_yasp(bool p0=false) {
    typedef Dune::FieldVector<int,dim> iTupel;
    typedef Dune::FieldVector<double,dim> fTupel;
    typedef Dune::FieldVector<bool,dim> bTupel;

    std::cout << std::endl << "YaspGrid<" << dim << "," << refinement << ">";
    if (p0) std::cout << " periodic\n";
    std::cout << std::endl << std::endl;

    fTupel Len; Len = 1.0;
    iTupel s; s = 1;
    bTupel p; p = false;
    p[0] = p0;
    int overlap = 1;

#if HAVE_MPI
    Dune::YaspGrid<dim> grid(MPI_COMM_WORLD,Len,s,p,overlap);
#else
    Dune::YaspGrid<dim> grid(Len,s,p,overlap);
#endif
    grid.globalRefine(refinement);

    // Test the interface
    Dune::GridInterfaceEuler<Dune::YaspGrid<dim> > gie(grid);
    test_evaluator<dim>(gie);
}


//-----------------------------------------------------------------------------
template <int refinement>
void check_cpgrid()
//-----------------------------------------------------------------------------
{
    std::cout << '\n' << "CpGrid<" << refinement << ">\n" << std::endl;

    Dune::CpGrid grid;
    Dune::array<int   , 3> dims;
    std::fill(dims.begin(), dims.end(), 1 << refinement);
    Dune::array<double, 3> cell_sz;
    std::fill(cell_sz.begin(), cell_sz.end(), 1.0 / (1 << refinement));

    grid.createCartesian(dims, cell_sz);

    // Test the interface
    Dune::GridInterfaceEuler<Dune::CpGrid> gie(grid);
    test_evaluator<3>(gie);
#if 0
    test_flowsolver<3>(gie);
#endif
}


int main (int argc , char **argv) {
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
