// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATSCALARDIVEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATSCALARDIVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatScalarDivExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatScalarDivExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isMatScalarDivExpr_backend(const volatile MatScalarDivExpr<MT> *);

FalseType isMatScalarDivExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a matrix/scalar division expression
//        template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a matrix/scalar division
// expression template. In order to qualify as a valid matrix/scalar division expression template,
// the given type has to derive publicly from the MatScalarDivExpr base class. In case the given
// type is a valid matrix/scalar division expression template, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsMatScalarDivExpr : public decltype(isMatScalarDivExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatScalarDivExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatScalarDivExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatScalarDivExpr type trait.
// \ingroup math_type_traits
//
// The IsMatScalarDivExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatScalarDivExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatScalarDivExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatScalarDivExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatScalarDivExpr_v = IsMatScalarDivExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
