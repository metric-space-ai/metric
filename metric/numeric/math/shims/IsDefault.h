// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ISDEFAULT_H
#define METRIC_NUMERIC_MATH_SHIMS_ISDEFAULT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>
#include <metric/numeric/math/Accuracy.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/typetraits/IsSIMDPack.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  ISDEFAULT SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the given value/object is in default state.
// \ingroup math_shims
//
// \param v The value/object to be tested for its default state.
// \return \a true in case the given value/object is in its default state, \a false otherwise.
//
// The \a isDefault shim represents an abstract interface for testing a value/object whether
// it is in its default state or not. In case the value/object is in its default state, the
// function returns \a true, otherwise it returns \a false. For integral built-in data types,
// the function returns \a true in case the current value is zero:

   \code
   int i1 = 0;  // isDefault( i1 ) returns true
   int i2 = 1;  // isDefault( i2 ) returns false
   \endcode

// For floating point built-in data types, the function by default uses relaxed semantics and
// returns \a true in case the current value is close to zero within a certain accuracy:

   \code
   double d1 = 0.0;   // isDefault( d1 ) returns true
   double d2 = 1E-9;  // isDefault( d2 ) returns true since d2 is below 1E-8
   double d3 = 1.0;   // isDefault( d3 ) returns false
   \endcode

// Optionally, it is possible to switch between relaxed semantics (mtrc::numeric::relaxed) and strict
// semantics (mtrc::numeric::strict). In case of strict semantics, for floating point built-in data types
// the function returns \a true in case the current value is exactly zero:

   \code
					  // isDefault<strict>( ... ) | isDefault<relaxed>( ... )
   double d1 = 0.0;   //    true                  |    true
   double d2 = 1E-9;  //    false (not 0.0)       |    true (below 1E-8)
   double d3 = 1.0;   //    false                 |    false
   \endcode
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename Type // Type of the given value/object
		  ,
		  EnableIf_t<IsScalar_v<Type> || IsSIMDPack_v<Type>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE bool isDefault(const Type &v) noexcept(IsBuiltin_v<Type>)
{
	return v == Type();
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the given single precision floating point value is zero.
// \ingroup math_shims
//
// \param v The single precision floating point value to be tested for zero.
// \return \a true in case the given value is zero, \a false otherwise.
//
// This overload of the \a isDefault shim tests whether the given single precision floating point
// value is exactly zero or within an epsilon range to zero. In case the value is zero or close
// to zero the function returns \a true, otherwise it returns \a false.
*/
template <RelaxationFlag RF> // Relaxation flag
METRIC_NUMERIC_ALWAYS_INLINE bool isDefault(float v) noexcept
{
	if (RF == relaxed)
		return std::fabs(v) <= accuracy;
	else
		return v == 0.0F;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the given double precision floating point value is zero.
// \ingroup math_shims
//
// \param v The double precision floating point value to be tested for zero.
// \return \a true in case the given value is zero, \a false otherwise.
//
// This overload of the \a isDefault shim tests whether the given double precision floating point
// value is exactly zero or within an epsilon range to zero. In case the value is zero or close
// to zero the function returns \a true, otherwise it returns \a false.
*/
template <RelaxationFlag RF> // Relaxation flag
METRIC_NUMERIC_ALWAYS_INLINE bool isDefault(double v) noexcept
{
	if (RF == relaxed)
		return std::fabs(v) <= accuracy;
	else
		return v == 0.0;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the given extended precision floating point value is zero.
// \ingroup math_shims
//
// \param v The extended precision floating point value to be tested for zero.
// \return \a true in case the given value is zero, \a false otherwise.
//
// This overload of the \a isDefault shim tests whether the given extended precision floating
// point value is exactly zero or within an epsilon range to zero. In case the value is zero or
// close to zero the function returns \a true, otherwise it returns \a false.
*/
template <RelaxationFlag RF> // Relaxation flag
METRIC_NUMERIC_ALWAYS_INLINE bool isDefault(long double v) noexcept
{
	if (RF == relaxed)
		return std::fabs(v) <= accuracy;
	else
		return v == 0.0L;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the given complex number is zero.
// \ingroup math_shims
//
// \param v The complex number to be tested for zero.
// \return \a true in case the given value is zero, \a false otherwise.
//
// This overload of the \a isDefault shim tests whether both the real and the imaginary part of
// the given complex number are exactly zero or within an epsilon range to zero. In case the both
// parts are zero or close to zero the function returns \a true, otherwise it returns \a false.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename T> // Value type of the complex number
METRIC_NUMERIC_ALWAYS_INLINE bool isDefault(const complex<T> &v) noexcept(IsBuiltin_v<T>)
{
	return isDefault<RF>(real(v)) && isDefault<RF>(imag(v));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the given value/object is in default state.
// \ingroup math_shims
//
// \param v The value/object to be tested for its default state.
// \return \a true in case the given value/object is in its default state, \a false otherwise.
//
// The \a isDefault shim represents an abstract interface for testing a value/object whether
// it is in its default state or not. In case the value/object is in its default state, the
// function returns \a true, otherwise it returns \a false. For integral built-in data types,
// the function returns \a true in case the current value is zero:

   \code
   int i1 = 0;  // isDefault( i1 ) returns true
   int i2 = 1;  // isDefault( i2 ) returns false
   \endcode

// For floating point built-in data types, the function by default uses relaxed semantics and
// returns \a true in case the current value is close to zero within a certain accuracy:

   \code
   double d1 = 0.0;   // isDefault( d1 ) returns true
   double d2 = 1E-9;  // isDefault( d2 ) returns true since d2 is below 1E-8
   double d3 = 1.0;   // isDefault( d3 ) returns false
   \endcode

// Optionally, it is possible to switch between relaxed semantics (mtrc::numeric::relaxed) and strict
// semantics (mtrc::numeric::strict). In case of strict semantics, for floating point built-in data types
// the function returns \a true in case the current value is exactly zero:

   \code
					  // isDefault<strict>( ... ) | isDefault<relaxed>( ... )
   double d1 = 0.0;   //    true                  |    true
   double d2 = 1E-9;  //    false (not 0.0)       |    true (below 1E-8)
   double d3 = 1.0;   //    false                 |    false
   \endcode
*/
template <typename Type> // Type of the given value/object
METRIC_NUMERIC_ALWAYS_INLINE bool isDefault(const Type &v) noexcept(IsBuiltin_v<Type>)
{
	return isDefault<relaxed>(v);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
