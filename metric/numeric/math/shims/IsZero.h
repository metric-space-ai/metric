// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ISZERO_H
#define METRIC_NUMERIC_MATH_SHIMS_ISZERO_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/EnableIf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  ISZERO SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the given value/object represents the numeric value 0.
// \ingroup math_shims
//
// \param v The value to be tested.
// \return \a true in case the given value is 0, \a false otherwise.
//
// The \a isZero shim provides an abstract interface for testing a value/object of any type
// whether it represents the numeric value 0. In case the value/object is 0, the function
// returns \a true, otherwise it returns \a false:

   \code
   int i1 = 0;  // isZero( i1 ) returns true
   int i2 = 1;  // isZero( i2 ) returns false
   \endcode

// For floating point built-in data types, the function by default uses relaxed semantics and
// returns \a true in case the current value is close to zero within a certain accuracy:

   \code
   double d1 = 0.0;                  // isZero( d1 ) returns true
   double d2 = 1E-9;                 // isZero( d2 ) returns true since d2 is below 1E-8
   double d3 = 1.0;                  // isZero( d3 ) returns false
   complex<double> c1( 0.0, 0.0  );  // isZero( c1 ) returns true
   complex<double> c2( 0.0, 1E-9 );  // isZero( c2 ) returns true since imag(c2) is below 1E-8
   complex<double> c3( 1.0, 0.0  );  // isZero( c3 ) returns false
   \endcode

// Optionally, it is possible to switch between relaxed semantics (mtrc::numeric::relaxed) and strict
// semantics (mtrc::numeric::strict). In case of strict semantics, for floating point built-in data types
// the function returns \a true in case the current value is exactly zero:

   \code
									 // isZero<strict>( ... ) | isZero<relaxed>( ... )
   double d1 = 0.0;                  //    true               |    true
   double d2 = 1E-9;                 //    false (not 0.0)    |    true (below 1E-8)
   double d3 = 1.0;                  //    false              |    false
   complex<double> c1( 0.0, 0.0  );  //    true               |    true
   complex<double> c2( 0.0, 1E-9 );  //    false              |    true (below 1E-8)
   complex<double> c3( 1.0, 0.0  );  //    false              |    false
   \endcode
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename Type // Type of the given value/object
		  ,
		  EnableIf_t<IsScalar_v<Type>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE bool isZero(const Type &v) noexcept
{
	return isDefault<RF>(v);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the given value/object represents the numeric value 0.
// \ingroup math_shims
//
// \param v The value to be tested.
// \return \a true in case the given value is 0, \a false otherwise.
//
// The \a isZero shim provides an abstract interface for testing a value/object of any type
// whether it represents the numeric value 0. In case the value/object is 0, the function
// returns \a true, otherwise it returns \a false:

   \code
   int i1 = 0;  // isZero( i1 ) returns true
   int i2 = 1;  // isZero( i2 ) returns false
   \endcode

// For floating point built-in data types, the function by default uses relaxed semantics and
// returns \a true in case the current value is close to zero within a certain accuracy:

   \code
   double d1 = 0.0;                  // isZero( d1 ) returns true
   double d2 = 1E-9;                 // isZero( d2 ) returns true since d2 is below 1E-8
   double d3 = 1.0;                  // isZero( d3 ) returns false
   complex<double> c1( 0.0, 0.0  );  // isZero( c1 ) returns true
   complex<double> c2( 0.0, 1E-9 );  // isZero( c2 ) returns true since imag(c2) is below 1E-8
   complex<double> c3( 1.0, 0.0  );  // isZero( c3 ) returns false
   \endcode

// Optionally, it is possible to switch between relaxed semantics (mtrc::numeric::relaxed) and strict
// semantics (mtrc::numeric::strict). In case of strict semantics, for floating point built-in data types
// the function returns \a true in case the current value is exactly zero:

   \code
									 // isZero<strict>( ... ) | isZero<relaxed>( ... )
   double d1 = 0.0;                  //     true              |     true
   double d2 = 1E-9;                 //     false (not 0.0)   |     true (below 1E-8)
   double d3 = 1.0;                  //     false             |     false
   complex<double> c1( 0.0, 0.0  );  //     true              |     true
   complex<double> c2( 0.0, 1E-9 );  //     false             |     true (below 1E-8)
   complex<double> c3( 1.0, 0.0  );  //     false             |     false
   \endcode
*/
template <typename Type> // Type of the given value/object
METRIC_NUMERIC_ALWAYS_INLINE bool isZero(const Type &v) noexcept
{
	return isZero<relaxed>(v);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
