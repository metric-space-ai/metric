// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_UNDERLYINGSCALAR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_UNDERLYINGSCALAR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

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
template <typename, typename = void> struct UnderlyingScalarHelper;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Evaluation of the underlying scalar element type of a given data type.
// \ingroup math_type_traits
//
// This type trait evaluates the underlying scalar (i.e. non-vector and non-matrix) element type
// at the heart of the given data type \a T. For this purpose either a nested \a ElementType will
// be used. Examples:

   \code
   using Type1 = double;                                    // Built-in data type
   using Type2 = complex<float>;                            // Complex data type
   using Type3 = std::vector<short>;                        // std::vector with built-in element type
   using Type4 = StaticVector<int,3UL>;                     // Vector with built-in element type
   using Type5 = CompressedVector< DynamicVector<float> >;  // Vector with vector element type

   mtrc::numeric::UnderlyingScalar< Type1 >::Type  // corresponds to double
   mtrc::numeric::UnderlyingScalar< Type2 >::Type  // corresponds to complex<float>
   mtrc::numeric::UnderlyingScalar< Type3 >::Type  // corresponds to std::vector<short>
   mtrc::numeric::UnderlyingScalar< Type4 >::Type  // corresponds to int
   mtrc::numeric::UnderlyingScalar< Type5 >::Type  // corresponds to float
   \endcode

// Note that it is possible to add support for other data types that have an underlying scalar
// element type but do not provide a nested \a ElementType type by specializing the UnderlyingScalar
// class template.
*/
template <typename T> struct UnderlyingScalar {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename UnderlyingScalarHelper<RemoveCV_t<T>>::Type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the UnderlyingScalar type trait.
// \ingroup math_type_traits
//
// The UnderlyingScalar_t alias declaration provides a convenient shortcut to access the
// nested \a Type of the UnderlyingScalar class template. For instance, given the type \a T
// the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::UnderlyingScalar<T>::Type;
   using Type2 = mtrc::numeric::UnderlyingScalar_t<T>;
   \endcode
*/
template <typename T> using UnderlyingScalar_t = typename UnderlyingScalar<T>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the UnderlyingScalar type trait.
// \ingroup math_type_traits
*/
template <typename T, typename> struct UnderlyingScalarHelper {
	using Type = T;
};

template <typename T> struct UnderlyingScalarHelper<T, EnableIf_t<!IsSame_v<T, typename T::ElementType>>> {
	using Type = typename UnderlyingScalarHelper<typename T::ElementType>::Type;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
