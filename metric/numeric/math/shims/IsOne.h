// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ISONE_H
#define METRIC_NUMERIC_MATH_SHIMS_ISONE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/shims/Equal.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  ISONE SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the given value/object represents the numeric value 1.
// \ingroup math_shims
//
// \param v The value to be tested.
// \return \a true in case the given value is 1, \a false otherwise.
//
// The \a isOne shim provides an abstract interface for testing a value/object of any type
// whether it represents the numeric value 1. In case the value/object is 1, the function
// returns \a true, otherwise it returns \a false:

   \code
   int i1 = 0;  // isZero( i1 ) returns false
   int i2 = 1;  // isZero( i2 ) returns true
   \endcode

// For floating point built-in data types, the function by default uses relaxed semantics and
// returns \a true in case the current value is close to 1 within a certain accuracy:

   \code
   double d1 = 0.0;                      // isZero( d1 ) returns false
   double d2 = 1.0 + 1E-9;               // isZero( d2 ) returns true
   double d3 = 1.0;                      // isZero( d3 ) returns true
   complex<double> c1( 0.0, 0.0 );       // isZero( c1 ) returns false
   complex<double> c2( 1.0+1E-9, 0.0 );  // isZero( c2 ) returns true
   complex<double> c3( 1.0, 0.0 );       // isZero( c3 ) returns true
   complex<double> c4( 0.0, 1.0 );       // isZero( c4 ) returns false
   \endcode

// Optionally, it is possible to switch between relaxed semantics (mtrc::numeric::relaxed) and strict
// semantics (mtrc::numeric::strict). In case of strict semantics, for floating point built-in data types
// the function returns \a true in case the current value is exactly 1:

   \code
										 // isOne<strict>( ... ) | isOne<relaxed>( ... )
   double d1 = 0.0;                      //    false            |     false
   double d2 = 1.0 + 1E-9;               //    false (not 1.0)  |     true
   double d3 = 1.0;                      //    true             |     true
   complex<double> c1( 0.0, 0.0 );       //    false            |     false
   complex<double> c2( 1.0+1E-9, 0.0 );  //    false            |     true
   complex<double> c3( 1.0, 0.0 );       //    true             |     true
   complex<double> c4( 0.0, 1.0 );       //    false            |     false
   \endcode
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename Type // Type of the given value/object
		  ,
		  EnableIf_t<IsNumeric_v<Type>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE bool isOne(const Type &v) noexcept
{
	return equal<RF>(v, Type(1));
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Overload of the \a isOne function for non-numeric data types.
// \ingroup math_shims
//
// \param v The value/object to be tested.
// \return \a false.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename Type // Type of the given value/object
		  ,
		  DisableIf_t<IsNumeric_v<Type>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE bool isOne(const Type &v) noexcept
{
	MAYBE_UNUSED(v);

	return false;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the given value/object represents the numeric value 1.
// \ingroup math_shims
//
// \param v The value to be tested.
// \return \a true in case the given value is 1, \a false otherwise.
//
// The \a isOne shim provides an abstract interface for testing a value/object of any type
// whether it represents the numeric value 1. In case the value/object is 1, the function
// returns \a true, otherwise it returns \a false:

   \code
   int i1 = 0;  // isZero( i1 ) returns false
   int i2 = 1;  // isZero( i2 ) returns true
   \endcode

// For floating point built-in data types, the function by default uses relaxed semantics and
// returns \a true in case the current value is close to 1 within a certain accuracy:

   \code
   double d1 = 0.0;                      // isZero( d1 ) returns false
   double d2 = 1.0 + 1E-9;               // isZero( d2 ) returns true
   double d3 = 1.0;                      // isZero( d3 ) returns true
   complex<double> c1( 0.0, 0.0 );       // isZero( c1 ) returns false
   complex<double> c2( 1.0+1E-9, 0.0 );  // isZero( c2 ) returns true
   complex<double> c3( 1.0, 0.0 );       // isZero( c3 ) returns true
   complex<double> c4( 0.0, 1.0 );       // isZero( c4 ) returns false
   \endcode

// Optionally, it is possible to switch between relaxed semantics (mtrc::numeric::relaxed) and strict
// semantics (mtrc::numeric::strict). In case of strict semantics, for floating point built-in data types
// the function returns \a true in case the current value is exactly 1:

   \code
										 // isOne<strict>( ... ) | isOne<relaxed>( ... )
   double d1 = 0.0;                      //    false            |     false
   double d2 = 1.0 + 1E-9;               //    false (not 1.0)  |     true
   double d3 = 1.0;                      //    true             |     true
   complex<double> c1( 0.0, 0.0 );       //    false            |     false
   complex<double> c2( 1.0+1E-9, 0.0 );  //    false            |     true
   complex<double> c3( 1.0, 0.0 );       //    true             |     true
   complex<double> c4( 0.0, 1.0 );       //    false            |     false
   \endcode
*/
template <typename Type> METRIC_NUMERIC_ALWAYS_INLINE bool isOne(const Type &v) { return isOne<relaxed>(v); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
