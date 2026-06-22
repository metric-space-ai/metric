// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_UNDERLYINGELEMENT_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_UNDERLYINGELEMENT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/typetraits/RemoveCV.h>
#include <metric/numeric/util/typetraits/Void.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename, typename = void> struct UnderlyingElementHelper1;
template <typename, typename = void> struct UnderlyingElementHelper2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Evaluation of the element type of a given data type.
// \ingroup math_type_traits
//
// This type trait evaluates the underlying element type of the given data type \a T. If the given
// type provides a nested type \a ElementType, this type is reported as underlying element type
// type via the nested type \a Type. Else if the type provides a nested \a value_type, this type
// is reported as underlying element type. Else the given type itself reported as the underlying
// element type. Examples:

   \code
   using Type1 = double;                                    // Built-in data type
   using Type2 = complex<float>;                            // Complex data type
   using Type3 = std::vector<short>;                        // std::vector with built-in element type
   using Type4 = StaticVector<int,3UL>;                     // Vector with built-in element type
   using Type5 = CompressedMatrix< DynamicVector<float> >;  // Matrix with vector element type

   mtrc::numeric::UnderlyingElement< Type1 >::Type  // corresponds to double
   mtrc::numeric::UnderlyingElement< Type2 >::Type  // corresponds to float
   mtrc::numeric::UnderlyingElement< Type3 >::Type  // corresponds to short
   mtrc::numeric::UnderlyingElement< Type4 >::Type  // corresponds to int
   mtrc::numeric::UnderlyingElement< Type5 >::Type  // corresponds to DynamicVector<float>
   \endcode

// Note that it is possible to add support for other data types that have an underlying element
// type but do neither provide a nested \a ElementType nor \a value_type type by specializing
// the UnderlyingElement class template.
*/
template <typename T> struct UnderlyingElement {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename UnderlyingElementHelper1<RemoveCV_t<T>>::Type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the UnderlyingElement type trait.
// \ingroup math_type_traits
//
// The UnderlyingElement_t alias declaration provides a convenient shortcut to access the
// nested \a Type of the UnderlyingElement class template. For instance, given the type \a T
// the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::UnderlyingElement<T>::Type;
   using Type2 = mtrc::numeric::UnderlyingElement_t<T>;
   \endcode
*/
template <typename T> using UnderlyingElement_t = typename UnderlyingElement<T>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the UnderlyingElement type trait.
// \ingroup math_type_traits
*/
template <typename T, typename> struct UnderlyingElementHelper1 {
	using Type = typename UnderlyingElementHelper2<T>::Type;
};

template <typename T> struct UnderlyingElementHelper1<complex<T>> {
	using Type = T;
};

template <typename T> struct UnderlyingElementHelper1<T, Void_t<typename T::ElementType>> {
	using Type = typename T::ElementType;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the UnderlyingElement type trait.
// \ingroup math_type_traits
*/
template <typename T, typename> struct UnderlyingElementHelper2 {
	using Type = T;
};

template <typename T> struct UnderlyingElementHelper2<T, Void_t<typename T::value_type>> {
	using Type = typename T::value_type;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
