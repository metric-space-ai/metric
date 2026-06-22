// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISKRONEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISKRONEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/KronExpr.h>
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
/*!\brief Auxiliary helper functions for the IsKronExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isKronExpr_backend(const volatile KronExpr<MT> *);

FalseType isKronExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a Kronecker product expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a Kronecker product
// expression template. In order to qualify as a valid Kronecker product expression template,
// the given type has to derive publicly from the KronExpr base class. In case the given type
// is a valid Kronecker product expression template, the \a value member constant is set to
// \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsKronExpr : public decltype(isKronExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsKronExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsKronExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsKronExpr type trait.
// \ingroup math_type_traits
//
// The IsKronExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsKronExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsKronExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsKronExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsKronExpr_v = IsKronExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
