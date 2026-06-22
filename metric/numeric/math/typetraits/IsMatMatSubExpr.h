// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATMATSUBEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATMATSUBEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatMatSubExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatMatSubExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isMatMatSubExpr_backend(const volatile MatMatSubExpr<MT> *);

FalseType isMatMatSubExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a matrix/matrix subtraction expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a matrix/matrix
// subtraction expression template. In order to qualify as a valid matrix subtraction expression
// template, the given type has to derive publicly from the MatMatSubExpr base class. In case the
// given type is a valid matrix subtraction expression template, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsMatMatSubExpr : public decltype(isMatMatSubExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatMatSubExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatMatSubExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatMatSubExpr type trait.
// \ingroup math_type_traits
//
// The IsMatMatSubExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatMatSubExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatMatSubExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatMatSubExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatMatSubExpr_v = IsMatMatSubExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
