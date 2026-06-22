// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ISREAL_H
#define METRIC_NUMERIC_MATH_SHIMS_ISREAL_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/shims/IsZero.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  ISREAL SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the given value/object represents a real number.
// \ingroup math_shims
//
// \param v The value to be tested.
// \return \a true in case the given value represents a real number, \a false otherwise.
//
// The \a isReal shim provides an abstract interface for testing a value/object of any type
// whether it represents the a real number. In case the value/object is of built-in type, the
// function returns \a true:

   \code
   int    i = 1;    // isReal( i ) returns true
   double d = 1.0;  // isReal( d ) returns true
   \endcode

// In case the value/object is of complex type, the function returns \a true if the imaginary
// part is close to 0 (relaxed semantics):

   \code
   complex<double> c1( 1.0, 0.0 );  // isReal( c1 ) returns true
   complex<double> c2( 0.0, 1.0 );  // isReal( c2 ) returns false
   \endcode

// Optionally, it is possible to switch between relaxed semantics (mtrc::numeric::relaxed) and strict
// semantics (mtrc::numeric::strict). In case of strict semantics, for complex types the function returns
// \a true in case the imaginary part is exactly zero:

   \code
									 // isReal<strict>( ... ) | isReal<relaxed>( ... )
   complex<double> c1( 1.0, 0.0  );  //    true               |    true
   complex<double> c2( 1.0, 1E-9 );  //    false              |    true (below 1E-8)
   complex<double> c3( 0.0, 1.0  );  //    false              |    false
   \endcode

// For all other types the function returns \a false.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename Type // Type of the given value
		  ,
		  EnableIf_t<IsScalar_v<Type>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE bool isReal(const Type &v) noexcept
{
	MAYBE_UNUSED(v);

	return IsBuiltin_v<Type>;
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Overload of the \a isReal function for complex data types.
// \ingroup math_shims
//
// \param v The complex number to be tested.
// \return \a true in case the imaginary part is equal to 0, \a false if not.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename Type> // Type of the given value
METRIC_NUMERIC_ALWAYS_INLINE bool isReal(const complex<Type> &v) noexcept(IsBuiltin_v<Type>)
{
	return IsBuiltin_v<Type> && isZero<RF>(v.imag());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the given value/object represents a real number.
// \ingroup math_shims
//
// \param v The value to be tested.
// \return \a true in case the given value represents a real number, \a false otherwise.
//
// The \a isReal shim provides an abstract interface for testing a value/object of any type
// whether it represents the a real number. In case the value/object is of built-in type, the
// function returns \a true. In case the value/object is of complex type, the function returns
// \a true if the imaginary part is equal to 0. Otherwise it returns \a false.

   \code
   int    i = 1;                      // isReal( i ) returns true
   double d = 1.0;                    // isReal( d ) returns true

   complex<double> c1( 1.0, 0.0 );    // isReal( c1 ) returns true
   complex<double> c2( 0.0, 1.0 );    // isReal( c2 ) returns false

   mtrc::numeric::DynamicVector<int> vec;     // isReal( vec ) returns false
   mtrc::numeric::DynamicMatrix<double> mat;  // isReal( mat ) returns false
   \endcode
*/
template <typename Type> METRIC_NUMERIC_ALWAYS_INLINE bool isReal(const Type &v) noexcept { return isReal<relaxed>(v); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
