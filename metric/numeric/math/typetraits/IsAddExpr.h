// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISADDEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISADDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/AddExpr.h>
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
/*!\brief Auxiliary helper functions for the IsAddExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isAddExpr_backend(const volatile AddExpr<U> *);

FalseType isAddExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is an addition expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is an addition expression
// template (i.e. an expression representing a vector addition or a matrix addition). In order
// to qualify as a valid addition expression template, the given type has to derive publicly
// from the AddExpr base class. In case the given type is a valid addition expression template,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsAddExpr : public decltype(isAddExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsAddExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsAddExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsAddExpr type trait.
// \ingroup math_type_traits
//
// The IsAddExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsAddExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsAddExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsAddExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsAddExpr_v = IsAddExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
