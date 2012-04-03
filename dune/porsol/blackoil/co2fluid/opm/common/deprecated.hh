#ifndef OPM_DEPRECATED_HH
#define OPM_DEPRECATED_HH

//! @addtogroup Common
//! @{
#if defined(DOXYGEN) or not defined(OPM_DEPRECATED)
//! Mark some entity as deprecated
/**
 * \code
#include <opm/common/deprecated.hh>
 * \endcode
 *
 * This is a preprocessor define which can be used to mark functions,
 * typedefs, enums and other stuff deprecated.  If something is marked
 * deprecated, users are advised to migrate to the new interface, since it
 * will probably be removed in the next release of Dune.
 *
 * OPM_DEPRECATED currently works with g++ only.  For other compilers it will
 * be defined empty.  This way the user will not get any deprecation warning,
 * but at least his code still compiles (well, until the next Dune release,
 * that is).
 *
 * Here are some examples how to mark different stuff deprecated:
 *  - Classes
 *    \code
class OPM_DEPRECATED Class {}; // 1)
class Class {} OPM_DEPRECATED; // 2)
 *    \endcode
 *    Both forms do not work properly with g++-4.1: no deprecation warning
 *    will be given, although the code still compiles.  1) should be preferred
 *    over 2) since 2) does not work with clang++-1.1 (again, no warning given
 *    but code still compiles)
 *  - Template classes
 *    \code
template<class T>
class OPM_DEPRECATED Class {}; // 1)
template<class T>
class Class {} OPM_DEPRECATED; // 2)
 *    \endcode
 *    This works works with g++-4.3, g++-4.4 and g++-4.5 only, g++-4.1 and
 *    clang++ compile the code without warning in both cases.  Furthermore,
 *    the warning is only triggered when copying an object of that template
 *    class, neither making a typedef nor simply creating such an object emit
 *    the warning.  It is thus recommended that some essential class member be
 *    marked deprecated as well, if possible.
 *  - Member constants
 *    \code
template<typename T> struct Class {
  static const int c0 OPM_DEPRECATED = 0;
  static const int OPM_DEPRECATED c1 = 1;
};
 *    \endcode
 *    Works with g++-4.1, g++-4.3, g++-4.4, g++-4.5.  No warning but clean
 *    compile with clang++-1.1.
 *  - Member enumerators
 *    \code
template<typename T> struct Class {
  enum enumeration { enumerator = 0 };
};
 *    \endcode
 *    No form of deprecation is known that does not trigger an error on most
 *    compilers.
 *  - Member functions
 *    \code
template<typename T> struct Class {
  void frob() OPM_DEPRECATED {}
};
template<typename T> struct Class {
  void OPM_DEPRECATED frob() {}
};
template<typename T> struct Class {
  OPM_DEPRECATED void frob() {}
};
 *    \endcode
 */
#define OPM_DEPRECATED
#endif
//! @}

#endif
