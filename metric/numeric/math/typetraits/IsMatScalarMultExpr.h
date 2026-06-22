// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATSCALARMULTEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATSCALARMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatScalarMultExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatScalarMultExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isMatScalarMultExpr_backend(const volatile MatScalarMultExpr<MT> *);

FalseType isMatScalarMultExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a matrix/scalar multiplication expression
//        template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a matrix/scalar
// multiplication expression template. In order to qualify as a valid matrix/scalar
// multiplication expression template, the given type has to derive publicly from the
// MatScalarMultExpr base class. In case the given type is a valid matrix/scalar multiplication
// expression template, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.
*/
template <typename T> struct IsMatScalarMultExpr : public decltype(isMatScalarMultExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatScalarMultExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatScalarMultExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatScalarMultExpr type trait.
// \ingroup math_type_traits
//
// The IsMatScalarMultExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatScalarMultExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatScalarMultExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatScalarMultExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatScalarMultExpr_v = IsMatScalarMultExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
