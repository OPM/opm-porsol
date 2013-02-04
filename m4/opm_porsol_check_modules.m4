dnl -*- autoconf -*-

# OPM_PORSOL_CHECK_MODULES(NAME, HEADER, SYMBOL)
#
# THIS MACRO IS JUST A COPY OF DUNE_CHECK_MODULES WITH THE REQUIREMENT
# THAT ALL HEADERS MUST RESIDE IN $MODULE_ROOT/dune REMOVED. REMOVE
# THIS MACRO AS SOON AS DUNE DOES NOT ENFORCE THIS ANYMORE.
#
# Generic check for dune modules.  This macro should not be used directly, but
# in the modules m4/{module}.m4 in the {MODULE}_CHECK_MODULE macro.  The
# {MODULE}_CHECK_MODULE macro knows the parameters to call this
# DUNE_CHECK_MODULES macro with, and it does not take any parameters itself,
# so it may be used with AC_REQUIRE.
#
# NAME   Name of the module, lowercase with dashes (like "dune-common").  The
#        value must be known when autoconf runs, so shell variables in the
#        value are not permissible.
#
# HEADER Header to check for.  The check will really be for <dune/{HEADER}>,
#        so the header must reside within a directory called "dune".
#
# SYMBOL Symbol to check for in the module's library.  If this argument is
#        empty or missing, it is assumed that the module does not provide a
#        library.  The value must be known when autoconf runs, so shell
#        variables in the value are not permissible.  This value is actually
#        handed to AC_TRY_LINK unchanged as the FUNCTION-BODY argument, so it
#        may contain more complex stuff than a simple symbol.
#
#        The name of the library is assumed to be the same as the module name,
#        with any occurance of "-" removed.  The path of the library is
#        obtained from pkgconfig for installed modules, or assumed to be the
#        directory "lib" within the modules root for non-installed modules.
#
# In the following, {module} is {NAME} with any "-" replaced by "_" and
# {MODULE} is the uppercase version of {module}.
#
# configure options:
#   --with-{NAME}
#
# configure/shell variables:
#   {MODULE}_ROOT, {MODULE}_LIBDIR
#   HAVE_{MODULE} (1 or 0)
#   with_{module} ("yes" or "no")
#   DUNE_CPPFLAGS, DUNE_LDFLAGS, DUNE_LIBS (adds the modules values here,
#         substitution done by DUNE_CHECK_ALL)
#   ALL_PKG_CPPFLAGS, ALL_PKG_LDFLAGS, ALL_PKG_LIBS (adds the modules values
#         here, substitution done by DUNE_CHECK_ALL)
#   {MODULE}_VERSION
#   {MODULE}_VERSION_MAJOR
#   {MODULE}_VERSION_MINOR
#   {MODULE}_VERSION_REVISION
#
# configure substitutions/makefile variables:
#   {MODULE}_CPPFLAGS, {MODULE}_LDFLAGS, {MODULE}_LIBS
#   {MODULE}_ROOT
#   {MODULE}_LIBDIR (only if modules provides a library)
#
# preprocessor defines:
#   HAVE_{MODULE} (1 or undefined)
#   {MODULE}_VERSION
#   {MODULE}_VERSION_MAJOR
#   {MODULE}_VERSION_MINOR
#   {MODULE}_VERSION_REVISION
#
# automake conditionals:
#   HAVE_{MODULE}
AC_DEFUN([OPM_PORSOL_CHECK_MODULES],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CXXCPP])
  AC_REQUIRE([PKG_PROG_PKG_CONFIG])
  AC_REQUIRE([DUNE_DISABLE_LIBCHECK])
  AC_REQUIRE([LT_OUTPUT])

  # ____DUNE_CHECK_MODULES_____ ($1)

  m4_pushdef([_dune_name], [$1])
  m4_pushdef([_dune_module], [m4_translit(_dune_name, [-], [_])])
  m4_pushdef([_dune_header], [$2])
  m4_pushdef([_dune_ldpath], [lib])
  m4_pushdef([_dune_lib],    [m4_translit(_dune_name, [-], [])])
  m4_pushdef([_dune_symbol], [$3])
  m4_pushdef([_DUNE_MODULE], [m4_toupper(_dune_module)])

  # switch tests to c++
  AC_LANG_PUSH([C++])

  # the usual option...
  AC_ARG_WITH(_dune_name,
    AS_HELP_STRING([--with-_dune_name=PATH],[_dune_module directory]))

  # backup of flags
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"
  ac_save_LDFLAGS="$LDFLAGS"
  CPPFLAGS=""
  LIBS=""

  ##
  ## Where is the module $1?
  ##
  
  AC_MSG_CHECKING([for $1 installation or source tree])

  # is a directory set?
  AS_IF([test -z "$with_[]_dune_module"],[
    #
    # search module $1 via pkg-config
    #
    with_[]_dune_module="global installation"
    AS_IF([test -z "$PKG_CONFIG"],[
      AC_MSG_RESULT([failed])
      AC_MSG_NOTICE([could not search for module _dune_name])
      AC_MSG_ERROR([pkg-config is required for using installed modules])
    ])
    AS_IF(AC_RUN_LOG([$PKG_CONFIG --exists --print-errors "$1"]),[
      _dune_cm_CPPFLAGS="`$PKG_CONFIG --cflags _dune_name`" 2>/dev/null
      _DUNE_MODULE[]_ROOT="`$PKG_CONFIG --variable=prefix _dune_name`" 2>/dev/null 
      _DUNE_MODULE[]_VERSION="`$PKG_CONFIG --modversion _dune_name`" 2>/dev/null
      _dune_cm_LDFLAGS=""
      ifelse(_dune_symbol,,
        [_DUNE_MODULE[]_LIBDIR=""
         _dune_cm_LIBS=""],
        [_DUNE_MODULE[]_LIBDIR=`$PKG_CONFIG --variable=libdir _dune_name 2>/dev/null`
         _dune_cm_LIBS="-L$_DUNE_MODULE[]_LIBDIR -l[]_dune_lib"])
      HAVE_[]_DUNE_MODULE=1
      AC_MSG_RESULT([global installation in $_DUNE_MODULE[]_ROOT])
    ],[
      HAVE_[]_DUNE_MODULE=0
      AC_MSG_RESULT([not found])
    ])
  ],[
    #
    # path for module $1 is specified via command line
    #
    AS_IF([test -d "$with_[]_dune_module"],[
      # expand tilde / other stuff
      _DUNE_MODULE[]_ROOT=`cd $with_[]_dune_module && pwd`

      # expand search path (otherwise empty CPPFLAGS)
      AS_IF([test -d "$_DUNE_MODULE[]_ROOT/include/dune"],[
        # Dune was installed into directory given by with-dunecommon
        _dune_cm_CPPFLAGS="-I$_DUNE_MODULE[]_ROOT/include"
        _DUNE_MODULE[]_BUILDDIR=_DUNE_MODULE[]_ROOT
        _DUNE_MODULE[]_VERSION="`PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$_DUNE_MODULE[]_ROOT/lib/pkgconfig $PKG_CONFIG --modversion _dune_name`" 2>/dev/null
      ],[
        _DUNE_MODULE[]_SRCDIR=$_DUNE_MODULE[]_ROOT
        # extract src and build path from Makefile, if found
	    AS_IF([test -f $_DUNE_MODULE[]_ROOT/Makefile],[
          _DUNE_MODULE[]_SRCDIR="`sed -ne '/^abs_top_srcdir = /{s/^abs_top_srcdir = //; p;}' $_DUNE_MODULE[]_ROOT/Makefile`"
		])
        _dune_cm_CPPFLAGS="-I$_DUNE_MODULE[]_SRCDIR"
        _DUNE_MODULE[]_VERSION="`grep Version $_DUNE_MODULE[]_SRCDIR/dune.module | sed -e 's/^Version: *//'`" 2>/dev/null
      ])
      _dune_cm_LDFLAGS=""
      ifelse(_dune_symbol,,
        [_DUNE_MODULE[]_LIBDIR=""
         _dune_cm_LIBS=""],
        [_DUNE_MODULE[]_LIBDIR="$_DUNE_MODULE[]_ROOT/lib"
         _dune_cm_LIBS="-L$_DUNE_MODULE[]_LIBDIR -l[]_dune_lib"])
      # set expanded module path
      with_[]_dune_module="$_DUNE_MODULE[]_ROOT"
      HAVE_[]_DUNE_MODULE=1
      AC_MSG_RESULT([found in $_DUNE_MODULE[]_ROOT])
    ],[
      HAVE_[]_DUNE_MODULE=0
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([_dune_name-directory $with_[]_dune_module does not exist])
    ])
  ])

  CPPFLAGS="$ac_save_CPPFLAGS $DUNE_CPPFLAGS $_dune_cm_CPPFLAGS"
  ##  
  ## check for an arbitrary header
  ##
  AS_IF([test "$HAVE_[]_DUNE_MODULE" != "1"],[
    AC_CHECK_HEADER([[]_dune_header],
      [HAVE_[]_DUNE_MODULE=1], [HAVE_[]_DUNE_MODULE=0])
  ])    

  AS_IF([test "$HAVE_[]_DUNE_MODULE" != "1"],[
    AC_MSG_WARN([$_DUNE_MODULE[]_ROOT does not seem to contain a valid _dune_name (dune/[]_dune_header not found)])
  ])
  
  ##
  ## check for lib (if lib name was provided)
  ##
  ifelse(_dune_symbol,,
    AC_MSG_NOTICE([_dune_name does not provide libs]),

    AS_IF([test "x$enable_dunelibcheck" = "xno"],[
      AC_MSG_WARN([library check for _dune_name is disabled. DANGEROUS!])
    ],[
      AS_IF([test "x$HAVE_[]_DUNE_MODULE" = "x1"],[

        # save current LDFLAGS
        ac_save_CXX="$CXX"
        HAVE_[]_DUNE_MODULE=0

        # define LTCXXLINK like it will be defined in the Makefile
        CXX="./libtool --tag=CXX --mode=link $ac_save_CXX"

        # use module LDFLAGS
        LDFLAGS="$ac_save_LDFLAGS $DUNE_LDFLAGS $_dune_cm_LDFLAGS"
        LIBS="$_dune_cm_LIBS $DUNE_LIBS $LIBS"

        AC_MSG_CHECKING([for lib[]_dune_lib])

        AC_TRY_LINK(dnl
          [#]include<_dune_header>,
          _dune_symbol,
          [
            AC_MSG_RESULT([yes])
            HAVE_[]_DUNE_MODULE=1
          ],[
            AC_MSG_RESULT([no])
            HAVE_[]_DUNE_MODULE=0
            AS_IF([test -n "$_DUNE_MODULE[]_ROOT"],[
             AC_MSG_WARN([$with_[]_dune_module does not seem to contain a valid _dune_name (failed to link with lib[]_dune_lib[].la)])
            ])
          ]
        )
      ])

      # reset variables
      CXX="$ac_save_CXX"
    ])
  )

  # did we succeed?
  AS_IF([test "x$HAVE_[]_DUNE_MODULE" = "x1"],[
    # add the module's own flags and libs to the modules and the global
    # variables
    DUNE_ADD_MODULE_DEPS(m4_defn([_dune_name]), m4_defn([_dune_name]),
        [$_dune_cm_CPPFLAGS], [$_dune_cm_LDFLAGS], [$_dune_cm_LIBS])

    # set variables for our modules
    AC_SUBST(_DUNE_MODULE[]_CPPFLAGS, "$_DUNE_MODULE[]_CPPFLAGS")
    AC_SUBST(_DUNE_MODULE[]_LDFLAGS, "$_DUNE_MODULE[]_LDFLAGS")
    AC_SUBST(_DUNE_MODULE[]_LIBS, "$_DUNE_MODULE[]_LIBS")
    AC_SUBST(_DUNE_MODULE[]_ROOT, "$_DUNE_MODULE[]_ROOT")
    ifelse(m4_defn([_dune_symbol]),,
      [],
      [AC_SUBST(_DUNE_MODULE[]_LIBDIR)
    ])
    AC_DEFINE(HAVE_[]_DUNE_MODULE, 1, [Define to 1 if] _dune_name [was found])

    DUNE_PARSE_MODULE_VERSION(_dune_name, $_DUNE_MODULE[]_VERSION)

    # set DUNE_* variables
    # This should actually be unneccesary, but I'm keeping it in here for now
    # for backward compatibility
    DUNE_LDFLAGS="$DUNE_LDFLAGS $_DUNE_MODULE[]_LDFLAGS"
    DUNE_LIBS="$_DUNE_MODULE[]_LIBS $DUNE_LIBS"
    
    with_[]_dune_module="yes"
  ],[
    with_[]_dune_module="no"
  ])

  AM_CONDITIONAL(HAVE_[]_DUNE_MODULE, test x$HAVE_[]_DUNE_MODULE = x1)

  # reset previous flags
  CPPFLAGS="$ac_save_CPPFLAGS"
  LDFLAGS="$ac_save_LDFLAGS"
  LIBS="$ac_save_LIBS"

  # add this module to DUNE_SUMMARY
  DUNE_MODULE_ADD_SUMMARY_ENTRY(_dune_name)

  # remove local variables
  m4_popdef([_dune_name])
  m4_popdef([_dune_module])
  m4_popdef([_dune_header])
  m4_popdef([_dune_ldpath])
  m4_popdef([_dune_lib])
  m4_popdef([_dune_symbol])
  m4_popdef([_DUNE_MODULE])

  # restore previous language settings (leave C++)
  AC_LANG_POP([C++])
])
