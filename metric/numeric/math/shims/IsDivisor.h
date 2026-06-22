// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ISDIVISOR_H
#define METRIC_NUMERIC_MATH_SHIMS_ISDIVISOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/shims/Equal.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/EnableIf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  ISDIVISOR SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the given value/object is a valid divisor.
// \ingroup math_shims
//
// \param v The value to be tested.
// \return \a true in case the given value is a valid divisor, \a false otherwise.
//
// The \a isDivisor shim provides an abstract interface for testing a value/object of any type
// whether it represents a valid divisor. In case the value/object can be used as divisor, the
// function returns \a true, otherwise it returns \a false.

   \code
   const int i1 = 1;                 // isDivisor( i1 ) returns true
   double    d1 = 0.1;               // isDivisor( d1 ) returns true
   complex<double> c1( 0.2, -0.1 );  // isDivisor( c1 ) returns true

   const int i2 = 0;                // isDivisor( i2 ) returns false
   double    d2 = 0.0;              // isDivisor( d2 ) returns false
   complex<double> c2( 0.0, 0.0 );  // isDivisor( c2 ) returns false
   \endcode
*/
template <typename Type, EnableIf_t<IsScalar_v<Type>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE bool isDivisor(const Type &v)
{
	return v != Type{};
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
