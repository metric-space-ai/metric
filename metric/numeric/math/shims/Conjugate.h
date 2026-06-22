// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_CONJUGATE_H
#define METRIC_NUMERIC_MATH_SHIMS_CONJUGATE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CONJ SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computing the conjugate of the given numeric value.
// \ingroup math_shims
//
// \param a The given numeric value.
// \return The complex conjugate of the given value.
//
// The \a conj shim represents an abstract interface for the computation of the complex conjugate
// of a numeric value. In case the given value is of complex type the function computes the
// complex conjugate by reversing the sign of the imaginary part:

   \code
   const mtrc::numeric::complex<double> a( 1.0, 2.0 );
   const mtrc::numeric::complex<double> b( conj( a ) );  // Results in ( 1, -2 )
   \endcode

// Values of built-in data type are considered complex numbers with an imaginary part of 0. Thus
// the returned value corresponds to the given value. For more information on complex conjugates,
// see
//
//       https://en.wikipedia.org/wiki/Complex_conjugate
*/
template <typename T, EnableIf_t<IsNumeric_v<T>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE constexpr T conj(const T &a) noexcept
{
	return a;
}
//*************************************************************************************************

//=================================================================================================
//
//  CONJUGATE SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief In-place conjugation of the given value/object.
// \ingroup math_shims
//
// \param a The given value/object to be conjugated.
// \return void
//
// The \a conjugate shim represents an abstract interface for the in-place conjugation of any
// given value/object. In case the given value is of complex type the function computes the
// complex conjugate by reversing the sign of the imaginary part:

   \code
   mtrc::numeric::complex<double> a( 1.0, 2.0 );
   conjugate( a );  // Results in ( 1, -2 )
   \endcode

// Values of other data types, such as all built-in data types, are considered complex numbers
// with an imaginary part of 0. Thus the returned value corresponds to the given value. For more
// information on complex conjugates, see
//
//       https://en.wikipedia.org/wiki/Complex_conjugate
*/
template <typename T> METRIC_NUMERIC_ALWAYS_INLINE void conjugate(T &a) noexcept(IsNumeric_v<T>) { a = conj(a); }
//*************************************************************************************************

//=================================================================================================
//
//  CSWAP SHIM
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend function of the \a cswap function for non-numeric data types.
// \ingroup math_shims
//
// \param a The first value/object to be swapped and conjugated.
// \param b The second value/object to be swapped and conjugated.
// \return void
*/
template <typename T, DisableIf_t<IsNumeric_v<T>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE void cswap_backend(T &a, T &b)
{
	using std::swap;

	swap(a, b);
	conjugate(a);
	conjugate(b);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend function of the \a cswap function for numeric data types.
// \ingroup math_shims
//
// \param a The first value to be swapped and conjugated.
// \param b The second value to be swapped and conjugated.
// \return void
*/
template <typename T, EnableIf_t<IsNumeric_v<T>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE void cswap_backend(T &a, T &b) noexcept
{
	const T tmp(a);
	a = conj(b);
	b = conj(tmp);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Swapping two conjugated values/objects.
// \ingroup math_shims
//
// \param a The first value/object to be swapped and conjugated.
// \param b The second value/object to be swapped and conjugated.
// \return void
//
// The \a cswap shim implements the most efficient way to swap and conjugate two values/objects.
// Semantically \a cswap is equivalent to the following sequence of operations:

   \code
   swap( a, b );
   conjugate( a );
   conjugate( b );
   \endcode
*/
template <typename T> METRIC_NUMERIC_ALWAYS_INLINE void cswap(T &a, T &b) noexcept(IsNumeric_v<T>)
{
	cswap_backend(a, b);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
