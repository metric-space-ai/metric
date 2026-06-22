// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASCLAMP_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASCLAMP_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/Void.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the HasClamp type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasClampHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasClampHelper type trait for types providing the clamp() operation.
// \ingroup math_type_traits
*/
template <typename T>
struct HasClampHelper<T, Void_t<decltype(clamp(std::declval<T>(), std::declval<T>(), std::declval<T>()))>>
	: public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the clamp() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the clamp() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoClamp {};  // Definition of a type without the clamp() operation

   mtrc::numeric::HasClamp< int >::value                  // Evaluates to 1
   mtrc::numeric::HasClamp< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasClamp< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasClamp< NoClamp >::value              // Evaluates to 0
   mtrc::numeric::HasClamp< NoClamp >::Type               // Results in FalseType
   mtrc::numeric::HasClamp< NoClamp >                     // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasClamp : public HasClampHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasClamp type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasClamp<T, EnableIf_t<IsVector_v<T>>> : public HasClamp<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasClamp type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasClamp<T, EnableIf_t<IsMatrix_v<T>>> : public HasClamp<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasClamp type trait.
// \ingroup math_type_traits
//
// The HasClamp_v variable template provides a convenient shortcut to access the nested \a value
// of the HasClamp class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasClamp<T>::value;
   constexpr bool value2 = mtrc::numeric::HasClamp_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasClamp_v = HasClamp<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
