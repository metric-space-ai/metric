// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_UNDERLYINGBUILTIN_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_UNDERLYINGBUILTIN_H
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
template <typename, typename = void> struct UnderlyingBuiltinHelper1;
template <typename, typename = void> struct UnderlyingBuiltinHelper2;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Evaluation of the underlying builtin element type of a given data type.
// \ingroup math_type_traits
//
// This type trait evaluates the underlying fundamental element type at the heart of the given
// data type \a T. For this purpose either a nested \a ElementType or a nested \a value_type will
// be used. Examples:

   \code
   using Type1 = double;                                    // Built-in data type
   using Type2 = complex<float>;                            // Complex data type
   using Type3 = std::vector<short>;                        // std::vector with built-in element type
   using Type4 = StaticVector<int,3UL>;                     // Vector with built-in element type
   using Type5 = CompressedVector< DynamicVector<float> >;  // Vector with vector element type

   mtrc::numeric::UnderlyingBuiltin< Type1 >::Type  // corresponds to double
   mtrc::numeric::UnderlyingBuiltin< Type2 >::Type  // corresponds to float
   mtrc::numeric::UnderlyingBuiltin< Type3 >::Type  // corresponds to short
   mtrc::numeric::UnderlyingBuiltin< Type4 >::Type  // corresponds to int
   mtrc::numeric::UnderlyingBuiltin< Type5 >::Type  // corresponds to float
   \endcode

// Note that it is possible to add support for other data types that have an underlying
// fundamental element type but do neither provide a nested \a ElementType nor \a value_type type
// by specializing the UnderlyingBuiltin class template.
*/
template <typename T> struct UnderlyingBuiltin {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename UnderlyingBuiltinHelper1<RemoveCV_t<T>>::Type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the UnderlyingBuiltin type trait.
// \ingroup math_type_traits
//
// The UnderlyingBuiltin_t alias declaration provides a convenient shortcut to access the
// nested \a Type of the UnderlyingBuiltin class template. For instance, given the type \a T
// the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::UnderlyingBuiltin<T>::Type;
   using Type2 = mtrc::numeric::UnderlyingBuiltin_t<T>;
   \endcode
*/
template <typename T> using UnderlyingBuiltin_t = typename UnderlyingBuiltin<T>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the UnderlyingBuiltin type trait.
// \ingroup math_type_traits
*/
template <typename T, typename> struct UnderlyingBuiltinHelper1 {
	using Type = typename UnderlyingBuiltinHelper2<T>::Type;
};

template <typename T> struct UnderlyingBuiltinHelper1<T, EnableIf_t<!IsSame_v<T, typename T::ElementType>>> {
	using Type = typename UnderlyingBuiltinHelper1<typename T::ElementType>::Type;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the UnderlyingBuiltin type trait.
// \ingroup math_type_traits
*/
template <typename T, typename> struct UnderlyingBuiltinHelper2 {
	using Type = T;
};

template <typename T> struct UnderlyingBuiltinHelper2<T, EnableIf_t<!IsSame_v<T, typename T::value_type>>> {
	using Type = typename UnderlyingBuiltinHelper1<typename T::value_type>::Type;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
