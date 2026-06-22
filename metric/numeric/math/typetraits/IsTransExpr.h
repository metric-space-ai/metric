// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISTRANSEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISTRANSEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/TransExpr.h>
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
/*!\brief Auxiliary helper functions for the IsTransExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isTransExpr_backend(const volatile TransExpr<U> *);

FalseType isTransExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a transposition expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a transposition expression
// template (i.e. an expression representing a vector transposition or a matrix transposition).
// In order to qualify as a valid transposition expression template, the given type has to derive
// publicly from the TransExpr base class. In case the given type is a valid transposition
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsTransExpr : public decltype(isTransExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsTransExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsTransExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsTransExpr type trait.
// \ingroup math_type_traits
//
// The IsTransExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsTransExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsTransExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsTransExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsTransExpr_v = IsTransExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
