// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISNOALIASEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISNOALIASEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/NoAliasExpr.h>
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
/*!\brief Auxiliary helper functions for the IsNoAliasExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isNoAliasExpr_backend(const volatile NoAliasExpr<U> *);

FalseType isNoAliasExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a no-alias expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a no-alias expression
// template. In order to qualify as a valid no-alias expression template, the given type has to
// derive publicly from the NoAliasExpr base class. In case the given type is a valid no-alias
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsNoAliasExpr : public decltype(isNoAliasExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsNoAliasExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsNoAliasExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNoAliasExpr type trait.
// \ingroup math_type_traits
//
// The IsNoAliasExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsNoAliasExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNoAliasExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsNoAliasExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsNoAliasExpr_v = IsNoAliasExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
