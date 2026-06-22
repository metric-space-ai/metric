// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISEXPANDEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISEXPANDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/ExpandExpr.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper functions for the IsExpandExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isExpandExpr_backend(const volatile ExpandExpr<U> *);

FalseType isExpandExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is an expansion expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is an expansion expression
// template. In order to qualify as a valid expansion expression template, the given type has
// to derive publicly from the ExpandExpr base class. In case the given type is a valid expansion
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsExpandExpr : public decltype(isExpandExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsExpandExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsExpandExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsExpandExpr type trait.
// \ingroup math_type_traits
//
// The IsExpandExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsExpandExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsExpandExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsExpandExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsExpandExpr_v = IsExpandExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
