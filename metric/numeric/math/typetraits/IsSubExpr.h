// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSUBEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSUBEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SubExpr.h>
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
/*!\brief Auxiliary helper functions for the IsSubExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isSubExpr_backend(const volatile SubExpr<U> *);

FalseType isSubExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a subtraction expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a subtraction expression
// template (i.e. an expression representing a vector subtraction or a matrix subtraction). In
// order to qualify as a valid subtraction expression template, the given type has to derive
// publicly from the SubExpr base class. In case the given type is a valid subtraction expression
// template, the \a value member constant is set to \a true, the nested type definition \a Type
// is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsSubExpr : public decltype(isSubExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSubExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsSubExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSubExpr type trait.
// \ingroup math_type_traits
//
// The IsSubExpr_v variable template provides a convenient shortcut to access the nested \a value
// of the IsSubExpr class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSubExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSubExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSubExpr_v = IsSubExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
