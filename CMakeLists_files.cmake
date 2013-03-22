# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

# This file sets up five lists:
#	MAIN_SOURCE_FILES     List of compilation units which will be included in
#	                      the library. If it isn't on this list, it won't be
#	                      part of the library. Please try to keep it sorted to
#	                      maintain sanity.
#
#	TEST_SOURCE_FILES     List of programs that will be run as unit tests.
#
#	TEST_DATA_FILES       Files from the source three that should be made
#	                      available in the corresponding location in the build
#	                      tree in order to run tests there.
#
#	EXAMPLE_SOURCE_FILES  Other programs that will be compiled as part of the
#	                      build, but which is not part of the library nor is
#	                      run as tests.
#
#	PUBLIC_HEADER_FILES   List of public header files that should be
#	                      distributed together with the library. The source
#	                      files can of course include other files than these;
#	                      you should only add to this list if the *user* of
#	                      the library needs it.

# originally generated with the command:
# find opm -name '*.c*' -a ! -wholename '*/twophase2/*' -printf '\t%p\n' | sort
list (APPEND MAIN_SOURCE_FILES
	opm/porsol/blackoil/fluid/BlackoilPVT.cpp
	opm/porsol/blackoil/fluid/MiscibilityDead.cpp
	opm/porsol/blackoil/fluid/MiscibilityLiveGas.cpp
	opm/porsol/blackoil/fluid/MiscibilityLiveOil.cpp
	opm/porsol/blackoil/fluid/MiscibilityProps.cpp
	opm/porsol/common/LinearSolverISTL.cpp
	)

# originally generated with the command:
# find tests -name '*.cpp' -a ! -wholename '*/not-unit/*' -printf '\t%p\n' | sort
list (APPEND TEST_SOURCE_FILES
	tests/common/boundaryconditions_test.cpp
	tests/common/matrix_test.cpp
	)

# originally generated with the command:
# find tests -name '*.xml' -a ! -wholename '*/not-unit/*' -printf '\t%p\n' | sort
list (APPEND TEST_DATA_FILES
	)

# originally generated with the command:
# find examples -name '*.c*' -a ! -name 'twophase2_test.cpp' -printf '\t%p\n' | sort
list (APPEND EXAMPLE_SOURCE_FILES
	examples/aniso_implicitcap_test.cpp
	examples/aniso_simulator_test.cpp
	examples/blackoil_sim_test.cpp
	examples/co2_blackoil_pvt.cpp
	examples/co2_sim_test.cpp
	examples/implicitcap_test.cpp
	examples/known_answer_test.cpp
	examples/mimetic_aniso_solver_test.cpp
	examples/mimetic_periodic_test.cpp
	examples/mimetic_solver_test.cpp
	examples/simulator_implicit_test.cpp
	examples/simulator_test.cpp
	)

# originally generated with the command:
# find opm -name '*.h*' -a ! -name '*-pch.hpp' -a ! -wholename '*/twophase2/*' -printf '\t%p\n' | sort
list (APPEND PUBLIC_HEADER_FILES
	opm/porsol/blackoil/BlackoilFluid.hpp
	opm/porsol/blackoil/BlackoilInitialization.hpp
	opm/porsol/blackoil/BlackoilSimulator.hpp
	opm/porsol/blackoil/BlackoilWells.hpp
	opm/porsol/blackoil/co2fluid/benchmark3co2tables.hh
	opm/porsol/blackoil/co2fluid/BlackoilCo2PVT.hpp
	opm/porsol/blackoil/co2fluid/opm/common/exceptions.hh
	opm/porsol/blackoil/co2fluid/opm/common/fixedlengthspline_.hh
	opm/porsol/blackoil/co2fluid/opm/common/math.hh
	opm/porsol/blackoil/co2fluid/opm/common/misc.hh
	opm/porsol/blackoil/co2fluid/opm/common/splinecommon_.hh
	opm/porsol/blackoil/co2fluid/opm/common/spline.hh
	opm/porsol/blackoil/co2fluid/opm/common/valgrind.hh
	opm/porsol/blackoil/co2fluid/opm/common/variablelengthspline_.hh
	opm/porsol/blackoil/co2fluid/opm/material/binarycoefficients/brine_co2.hh
	opm/porsol/blackoil/co2fluid/opm/material/binarycoefficients/fullermethod.hh
	opm/porsol/blackoil/co2fluid/opm/material/binarycoefficients/h2o_n2.hh
	opm/porsol/blackoil/co2fluid/opm/material/binarycoefficients/henryiapws.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/brine.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/ch4.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/co2.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/component.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/h2.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/h2o.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/iapws/common.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/iapws/region1.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/iapws/region2.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/iapws/region4.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/n2.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/nullcomponent.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/o2.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/oil.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/simpleco2.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/simplednapl.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/simpleh2o.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/tabulatedcomponent.hh
	opm/porsol/blackoil/co2fluid/opm/material/components/unit.hh
	opm/porsol/blackoil/co2fluid/opm/material/constants.hh
	opm/porsol/blackoil/co2fluid/opm/material/constraintsolvers/compositionfromfugacities.hh
	opm/porsol/blackoil/co2fluid/opm/material/constraintsolvers/computefromreferencephase.hh
	opm/porsol/blackoil/co2fluid/opm/material/constraintsolvers/immiscibleflash.hh
	opm/porsol/blackoil/co2fluid/opm/material/constraintsolvers/misciblemultiphasecomposition.hh
	opm/porsol/blackoil/co2fluid/opm/material/constraintsolvers/ncpflash.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/brookscorey.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/brookscoreyparams.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/efftoabslaw.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/efftoabslawparams.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/linearmaterial.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/linearmaterialparams.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/regularizedbrookscorey.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/regularizedbrookscoreyparams.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/regularizedlinearmaterial.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/regularizedlinearmaterialparams.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/regularizedvangenuchten.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/regularizedvangenuchtenparams.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/vangenuchten.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/2p/vangenuchtenparams.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/Mp/2padapter.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/Mp/Mpbrookscorey.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/Mp/Mpbrookscoreyparams.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/Mp/Mplinearmaterial.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidmatrixinteractions/Mp/Mplinearmaterialparams.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidstates/compositionalfluidstate.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidstates/immisciblefluidstate.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidstates/nonequilibriumfluidstate.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidsystems/1pfluidsystem.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidsystems/2pimmisciblefluidsystem.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidsystems/basefluidsystem.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidsystems/brine_co2_system.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidsystems/gasphase.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidsystems/h2on2fluidsystem.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidsystems/liquidphase.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidsystems/nullparametercache.hh
	opm/porsol/blackoil/co2fluid/opm/material/fluidsystems/parametercachebase.hh
	opm/porsol/blackoil/co2fluid/opm/material/idealgas.hh
	opm/porsol/blackoil/co2fluid/opm/material/settablephase.hh
	opm/porsol/blackoil/co2fluid/opm/old_material/tabulatedmaterial2.hh
	opm/porsol/blackoil/co2fluid/opm/old_material/tabulatedmaterial2hires.hh
	opm/porsol/blackoil/ComponentTransport.hpp
	opm/porsol/blackoil/fluid/BlackoilComponent.hpp
	opm/porsol/blackoil/fluid/BlackoilDefs.hpp
	opm/porsol/blackoil/fluid/BlackoilPVT.hpp
	opm/porsol/blackoil/fluid/FluidMatrixInteractionBlackoil.hpp
	opm/porsol/blackoil/fluid/FluidStateBlackoil.hpp
	opm/porsol/blackoil/fluid/MiscibilityDead.hpp
	opm/porsol/blackoil/fluid/MiscibilityLiveGas.hpp
	opm/porsol/blackoil/fluid/MiscibilityLiveOil.hpp
	opm/porsol/blackoil/fluid/MiscibilityProps.hpp
	opm/porsol/blackoil/fluid/MiscibilityWater.hpp
	opm/porsol/common/AbstractLinearSolver.hpp
	opm/porsol/common/BCRSMatrixBlockAssembler.hpp
	opm/porsol/common/blas_lapack.hpp
	opm/porsol/common/BoundaryConditions.hpp
	opm/porsol/common/BoundaryPeriodicity.hpp
	opm/porsol/common/fortran.hpp
	opm/porsol/common/GridInterfaceEuler.hpp
	opm/porsol/common/ImplicitTransportDefs.hpp
	opm/porsol/common/LinearSolverISTL.hpp
	opm/porsol/common/Matrix.hpp
	opm/porsol/common/MatrixInverse.hpp
	opm/porsol/common/PeriodicHelpers.hpp
	opm/porsol/common/ReservoirPropertyCapillaryAnisotropicRelperm.hpp
	opm/porsol/common/ReservoirPropertyCapillaryAnisotropicRelperm_impl.hpp
	opm/porsol/common/ReservoirPropertyCapillary.hpp
	opm/porsol/common/ReservoirPropertyCapillary_impl.hpp
	opm/porsol/common/ReservoirPropertyCommon.hpp
	opm/porsol/common/ReservoirPropertyCommon_impl.hpp
	opm/porsol/common/ReservoirPropertyFixedMobility.hpp
	opm/porsol/common/ReservoirPropertyTracerFluid.hpp
	opm/porsol/common/RockAnisotropicRelperm.hpp
	opm/porsol/common/Rock.hpp
	opm/porsol/common/Rock_impl.hpp
	opm/porsol/common/RockJfunc.hpp
	opm/porsol/common/setupBoundaryConditions.hpp
	opm/porsol/common/setupGridAndProps.hpp
	opm/porsol/common/SimpleRock.hpp
	opm/porsol/common/SimulatorBase.hpp
	opm/porsol/common/SimulatorTraits.hpp
	opm/porsol/common/SimulatorUtilities.hpp
	opm/porsol/common/SintefLegacyGridInterface.hpp
	opm/porsol/common/Wells.hpp
	opm/porsol/euler/CflCalculator.hpp
	opm/porsol/euler/EulerSolverTester.hpp
	opm/porsol/euler/EulerUpstream.hpp
	opm/porsol/euler/EulerUpstream_impl.hpp
	opm/porsol/euler/EulerUpstreamImplicit.hpp
	opm/porsol/euler/EulerUpstreamImplicit_impl.hpp
	opm/porsol/euler/EulerUpstreamResidual.hpp
	opm/porsol/euler/EulerUpstreamResidual_impl.hpp
	opm/porsol/euler/ImplicitCapillarity.hpp
	opm/porsol/euler/ImplicitCapillarity_impl.hpp
	opm/porsol/euler/MatchSaturatedVolumeFunctor.hpp
	opm/porsol/mimetic/IncompFlowSolverHybrid.hpp
	opm/porsol/mimetic/MimeticIPAnisoRelpermEvaluator.hpp
	opm/porsol/mimetic/MimeticIPEvaluator.hpp
	opm/porsol/mimetic/TpfaCompressible.hpp
	opm/porsol/mimetic/TpfaCompressibleAssembler.hpp
	)
