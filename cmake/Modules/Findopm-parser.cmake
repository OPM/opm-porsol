# Find the OPM Eclipse input parser.
#
# Set the cache variable OPM_PARSER_ROOT to the install location of the
# library, or OPM_ROOT to the parent directory of the build tree.
#
# If found, it sets these variables:
#
#	HAVE_OPM_PARSER              Defined if a test program compiled
#	OPM_PARSER_INCLUDE_DIRS      Header file directories
#	OPM_PARSER_LIBRARIES         Archives and shared objects

include (FindPackageHandleStandardArgs)

# variables to pass on to other packages
if (FIND_QUIETLY)
  set (OPM_PARSER_QUIET "QUIET")
else ()
  set (OPM_PARSER_QUIET "")
endif ()

# use lowercase versions of the variables if those are set
if (opm-parser_ROOT)
  set (OPM_PARSER_ROOT ${opm-parser_ROOT})
endif ()
if (opm_ROOT)
  set (OPM_ROOT ${opm_ROOT})
endif ()

# inherit "suite" root if not specifically set for this library
if ((NOT OPM_PARSER_ROOT) AND OPM_ROOT)
  set (OPM_PARSER_ROOT "${OPM_ROOT}/opm-parser")
endif ()

# if a root is specified, then don't search in system directories
# or in relative directories to this one
if (OPM_PARSER_ROOT)
  set (_no_default_path "NO_DEFAULT_PATH")
  set (_opm_parser_source "")
  set (_opm_parser_build "")
else ()
  set (_no_default_path "")
  set (_opm_parser_source
    "${PROJECT_SOURCE_DIR}/../opm-parser")
  set (_opm_parser_build
    "${PROJECT_BINARY_DIR}/../opm-parser"
    "${PROJECT_BINARY_DIR}/../opm-parser-build"
    "${PROJECT_BINARY_DIR}/../../opm-parser/build"
    "${PROJECT_BINARY_DIR}/../../opm-parser/cmake-build")
endif ()

# use this header as signature
find_path (OPM_PARSER_INCLUDE_DIR
  NAMES "opm/parser/eclipse/Parser/Parser.hpp"
  HINTS "${OPM_PARSER_ROOT}"
  PATHS ${_opm_parser_source}
  PATH_SUFFIXES "include"
  DOC "Path to OPM parser header files"
  ${_no_default_path} )

# find out the size of a pointer. this is required to only search for
# libraries in the directories relevant for the architecture
if (CMAKE_SIZEOF_VOID_P)
  math (EXPR _BITS "8 * ${CMAKE_SIZEOF_VOID_P}")
endif (CMAKE_SIZEOF_VOID_P)

# these libraries constitute the parser core
find_library (OPM_PARSER_LIBRARY
  NAMES "Parser"
  HINTS "${OPM_PARSER_ROOT}"
  PATHS ${_opm_parser_build}
  PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
                "opm/parser/eclipse"
  DOC "Path to OPM parser library archive/shared object files"
  ${_no_default_path} )

find_library (OPM_JSON_LIBRARY
  NAMES "opm-json"
  HINTS "${OPM_PARSER_ROOT}"
  PATHS ${_opm_parser_build}
  PATH_SUFFIXES "lib" "lib${_BITS}" "lib/${CMAKE_LIBRARY_ARCHITECTURE}"
                "opm/json"
  DOC "Path to OPM JSON library archive/shared object files"
  ${_no_default_path} )

# get the prerequisite CJSON library
if (NOT CJSON_FOUND)
  find_package (cjson REQUIRED ${OPM_PARSER_QUIET})
endif ()

# get the prerequisite Boost libraries
if (NOT Boost_FOUND)
  find_package(Boost 1.44.0
	COMPONENTS filesystem date_time system unit_test_framework REQUIRED ${OPM_PARSER_QUIET})
endif ()

# setup list of all required libraries to link with opm-parser. notice that
# we use the plural form to get *all* the libraries needed by cjson
set (OPM_PARSER_INCLUDE_DIRS
  ${OPM_PARSER_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS})
set (OPM_PARSER_LIBRARIES
  ${OPM_PARSER_LIBRARY}
  ${OPM_JSON_LIBRARY}
  ${CJSON_LIBRARIES}
  ${Boost_LIBRARIES})

# see if we can compile a minimum example
# CMake logical test doesn't handle lists (sic)
if (NOT (OPM_PARSER_INCLUDE_DIR MATCHES "-NOTFOUND"
	  OR OPM_PARSER_LIBRARIES MATCHES "-NOTFOUND"))
  include (CMakePushCheckState)
  include (CheckCSourceCompiles)
  cmake_push_check_state ()
  set (CMAKE_REQUIRED_INCLUDES ${OPM_PARSER_INCLUDE_DIRS})
  set (CMAKE_REQUIRED_LIBRARIES ${OPM_PARSER_LIBRARIES})

  check_cxx_source_compiles (
"#include <cstdlib>
#include <opm/parser/eclipse/Parser/Parser.hpp>

int main (void) {
   return EXIT_SUCCESS;
}" HAVE_OPM_PARSER)
  cmake_pop_check_state ()
else ()
  # clear the cache so the find probe is attempted again if files becomes
  # available (only upon a unsuccessful *compile* should we disable further
  # probing)
  set (HAVE_OPM_PARSER)
  unset (HAVE_OPM_PARSER CACHE)
endif ()

# if the test program didn't compile, but was required to do so, bail
# out now and display an error; otherwise limp on
set (OPM_PARSER_FIND_REQUIRED ${opm-parser_FIND_REQUIRED})
set (OPM_PARSER_FIND_QUIETLY ${opm-parser_FIND_QUIETLY})
find_package_handle_standard_args (OPM_PARSER
  DEFAULT_MSG
  OPM_PARSER_INCLUDE_DIRS OPM_PARSER_LIBRARIES HAVE_OPM_PARSER
  )
set (opm-parser_FOUND ${OPM_PARSER_FOUND})
