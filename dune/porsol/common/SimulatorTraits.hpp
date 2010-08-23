//===========================================================================
//
// File: SimulatorTraits.hpp
//
// Created: Mon Jul 19 15:48:15 2010
//
// Author(s): Atgeirr F Rasmussen <atgeirr@sintef.no>
//
// $Date$
//
// $Revision$
//
//===========================================================================

/*
  Copyright 2010 SINTEF ICT, Applied Mathematics.
  Copyright 2010 Statoil ASA.

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

#ifndef OPENRS_SIMULATORTRAITS_HEADER
#define OPENRS_SIMULATORTRAITS_HEADER


#include <dune/porsol/common/ReservoirPropertyCapillaryAnisotropicRelperm.hpp>
#include <dune/porsol/mimetic/MimeticIPAnisoRelpermEvaluator.hpp>
#include <dune/porsol/common/ReservoirPropertyCapillary.hpp>
#include <dune/porsol/mimetic/MimeticIPEvaluator.hpp>
#include <dune/porsol/mimetic/IncompFlowSolverHybrid.hpp>
#include <dune/porsol/euler/EulerUpstream.hpp>
#include <dune/porsol/euler/ImplicitCapillarity.hpp>

namespace Dune
{


    /// Traits policies for isotropic (scalar) relperm.
    struct Isotropic
    {
        /// The reservoir property type.
        template <int Dimension>
        struct ResProp
        {
            typedef ReservoirPropertyCapillary<Dimension> Type;
        };

        /// The inner product template.
        template <class GridInterface, class RockInterface>
        struct InnerProduct : public MimeticIPEvaluator<GridInterface, RockInterface>
        {
        };
    };


    /// Traits for upscaling with anisotropic relperm (tensorial) input.
    struct Anisotropic
    {
        /// The reservoir property type.
        template <int Dimension>
        struct ResProp
        {
            typedef ReservoirPropertyCapillaryAnisotropicRelperm<Dimension> Type;
        };

        /// The inner product template.
        template <class GridInterface, class RockInterface>
        struct InnerProduct : public MimeticIPAnisoRelpermEvaluator<GridInterface, RockInterface>
        {
        };
    };


    /// Traits for explicit transport.
    template <class IsotropyPolicy>
    struct Explicit
    {
        template <class GridInterface, class BoundaryConditions>
        struct TransportSolver
        {
            enum { Dimension = GridInterface::Dimension };
            typedef typename IsotropyPolicy::template ResProp<Dimension>::Type RP;
            typedef EulerUpstream<GridInterface,
                                  RP,
                                  BoundaryConditions> Type;
        };
    };


    /// Traits for implicit transport (solving for capillary pressure of steady state implicitly).
    template <class IsotropyPolicy>
    struct ImplicitCap
    {
        template <class GridInterface, class BoundaryConditions>
        struct TransportSolver
        {
            enum { Dimension = GridInterface::Dimension };
            typedef typename IsotropyPolicy::template ResProp<Dimension>::Type RP;
            typedef ImplicitCapillarity<GridInterface, RP, BoundaryConditions, IsotropyPolicy::template InnerProduct> Type;
        };
    };



    /// Combines the component traits into a single, parametrized type.
    template <class RelpermPolicy, template <class> class TransportPolicy>
    struct SimulatorTraits : public RelpermPolicy, TransportPolicy<RelpermPolicy>
    {
        /// The pressure/flow solver type.
        template <class GridInterface, class BoundaryConditions>
        struct FlowSolver
        {
            typedef IncompFlowSolverHybrid<GridInterface,
                                           typename RelpermPolicy::template ResProp<GridInterface::Dimension>::Type,
                                           BoundaryConditions,
                                           RelpermPolicy::template InnerProduct> Type;
        };
    };


} // namespace Dune



#endif // OPENRS_SIMULATORTRAITS_HEADER
