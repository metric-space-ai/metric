// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_UNDERLYINGNUMERIC_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_UNDERLYINGNUMERIC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsSame.h>
#include <metric/numeric/util/typetraits/RemoveCV.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename, typename = void> struct UnderlyingNumericHelper1;
template <typename, typename = void> struct UnderlyingNumericHelper2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Evaluation of the underlying scalar element type of a given data type.
// \ingroup math_type_traits
//
// This type trait evaluates the underlying numeric (fundamental or complex) element type at the
// heart of the given data type \a T. For this purpose either a nested \a ElementType or a nested
// \a value_type will be used. Examples:

   \code
   using Type1 = double;                                    // Built-in data type
   using Type2 = complex<float>;                            // Complex data type
   using Type3 = std::vector<short>;                        // std::vector with built-in element type
   using Type4 = StaticVector<int,3UL>;                     // Vector with built-in element type
   using Type5 = CompressedVector< DynamicVector<float> >;  // Vector with vector element type

   mtrc::numeric::UnderlyingScalar< Type1 >::Type  // corresponds to double
   mtrc::numeric::UnderlyingScalar< Type2 >::Type  // corresponds to complex<float>
   mtrc::numeric::UnderlyingScalar< Type3 >::Type  // corresponds to short
   mtrc::numeric::UnderlyingScalar< Type4 >::Type  // corresponds to int
   mtrc::numeric::UnderlyingScalar< Type5 >::Type  // corresponds to float
   \endcode

// Note that it is possible to add support for other data types that have an underlying scalar
// element type but do neither provide a nested \a ElementType nor \a value_type type by
// specializing the UnderlyingScalar class template.
*/
template <typename T> struct UnderlyingNumeric {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename UnderlyingNumericHelper1<RemoveCV_t<T>>::Type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the UnderlyingNumeric type trait.
// \ingroup math_type_traits
//
// The UnderlyingNumeric_t alias declaration provides a convenient shortcut to access the
// nested \a Type of the UnderlyingNumeric class template. For instance, given the type \a T
// the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::UnderlyingNumeric<T>::Type;
   using Type2 = mtrc::numeric::UnderlyingNumeric_t<T>;
   \endcode
*/
template <typename T> using UnderlyingNumeric_t = typename UnderlyingNumeric<T>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the UnderlyingNumeric type trait.
// \ingroup math_type_traits
*/
template <typename T, typename> struct UnderlyingNumericHelper1 {
	using Type = typename UnderlyingNumericHelper2<T>::Type;
};

template <typename T> struct UnderlyingNumericHelper1<complex<T>, void> {
	using Type = complex<T>;
};

template <typename T> struct UnderlyingNumericHelper1<T, EnableIf_t<!IsSame_v<T, typename T::ElementType>>> {
	using Type = typename UnderlyingNumericHelper1<typename T::ElementType>::Type;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the UnderlyingNumeric type trait.
// \ingroup math_type_traits
*/
template <typename T, typename> struct UnderlyingNumericHelper2 {
	using Type = T;
};

template <typename T> struct UnderlyingNumericHelper2<T, EnableIf_t<!IsSame_v<T, typename T::value_type>>> {
	using Type = typename UnderlyingNumericHelper1<typename T::value_type>::Type;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
