# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

###########################################################################
#                                                                         #
# Note: The bulk of the build system is located in the cmake/ directory.  #
#       This file only contains the specializations for this particular   #
#       project. Most likely you are interested in editing one of these   #
#       files instead:                                                    #
#                                                                         #
#       dune.module                              Name and version number  #
#       CMakeLists_files.cmake                   Path of source files     #
#       cmake/Modules/${project}-prereqs.cmake   Dependencies             #
#                                                                         #
###########################################################################

cmake_minimum_required (VERSION 2.8)

# additional search modules
find_path (OPM_MACROS_ROOT cmake/Modules/OpmInit.cmake
	PATHS ${opm-macros_ROOT}
	      ${CMAKE_CURRENT_SOURCE_DIR}
	DOC "Path to build system macros and scripts"
	NO_DEFAULT_PATH
	)
list (APPEND CMAKE_MODULE_PATH "${OPM_MACROS_ROOT}/cmake/Modules")

# not the same location as most of the other projects? this hook overrides
macro (dir_hook)
endmacro (dir_hook)

# project information is in dune.module. Read this file and set variables.
# we cannot generate dune.module since it is read by dunecontrol before
# the build starts, so it makes sense to keep the data there then.
include (OpmInit)

# list of prerequisites for this particular project; this is in a
# separate file (in cmake/Modules sub-directory) because it is shared
# with the find module
include (${project}-prereqs)

# read the list of components from this file (in the project directory);
# it should set various lists with the names of the files to include
include (CMakeLists_files.cmake)

macro (config_hook)
	opm_need_version_of ("dune-common")
	opm_need_version_of ("dune-istl")
	opm_need_version_of ("dune-grid")
endmacro (config_hook)

macro (prereqs_hook)
endmacro (prereqs_hook)

macro (sources_hook)
	if (NOT HAVE_CONSTEXPR)
		opm_disable_source (examples
			examples/co2_blackoil_pvt.cpp
			examples/sim_co2_impes.cpp
			examples/sim_steadystate_implicit.cpp
			)
	endif (NOT HAVE_CONSTEXPR)
endmacro (sources_hook)

macro (fortran_hook)
endmacro (fortran_hook)

macro (files_hook)
	find_dune_version ("dune" "grid")
	if ((DUNE_GRID_VERSION_MAJOR GREATER 2) OR
		 ((DUNE_GRID_VERSION_MAJOR EQUAL 2) AND
		(DUNE_GRID_VERSION_MINOR GREATER 2)))
		# "known_answer_test.cpp" uses grid interface features that aren't
		# supported in recent versions of Dune.
		list (REMOVE_ITEM EXAMPLE_SOURCE_FILES
			"examples/known_answer_test.cpp")
	endIf ()
endmacro (files_hook)

macro (tests_hook)
endmacro (tests_hook)

# all setup common to the OPM library modules is done here
include (OpmLibMain)

add_custom_target(check-commits
									COMMAND ${CMAKE_COMMAND}
													-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
													-DCMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
													-P ${PROJECT_SOURCE_DIR}/cmake/Scripts/CheckCommits.cmake)
