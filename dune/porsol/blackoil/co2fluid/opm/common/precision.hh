#ifndef OPM_PRECISION_HH
#define OPM_PRECISION_HH

#include <stdlib.h>

namespace Opm {
   
    /** 
	@addtogroup DenseMatVec 
	@{
     */

  /**
   * @brief Precisions for calculations with FieldMatrix and FieldVector.
   */
  template <class ctype = double>
  class FMatrixPrecision {
  public:
	//! return threshold to do pivoting
	static ctype pivoting_limit ()
	{
	  return _pivoting;
	}

	//! set pivoting threshold
	static void set_pivoting_limit (ctype pivthres)
	{
	  _pivoting = pivthres;
	}

	//! return threshold to declare matrix singular
	static ctype singular_limit ()
	{
	  return _singular;
	}

	//! set singular threshold
	static void set_singular_limit (ctype singthres)
	{
	  _singular = singthres;
	}

	//! return threshold to declare matrix singular
	static ctype absolute_limit ()
	{
	  return _absolute;
	}

	//! set singular threshold
	static void set_absolute_limit (ctype absthres)
	{
	  _absolute = absthres;
	}

  private:
	// just to demonstrate some state information
	static ctype _pivoting;
	static ctype _singular;
	static ctype _absolute;
  };

  template <class ctype>
  ctype FMatrixPrecision<ctype>::_pivoting = 1E-8;
  template <class ctype>
  ctype FMatrixPrecision<ctype>::_singular = 1E-14;
  template <class ctype>
  ctype FMatrixPrecision<ctype>::_absolute = 1E-80;
 
  /** @} end documentation */

} // end namespace

#endif
