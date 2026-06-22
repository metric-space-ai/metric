// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISVECTVECMAPEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISVECTVECMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/VecTVecMapExpr.h>
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
/*!\brief Auxiliary helper functions for the IsVecTVecMapExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isVecTVecMapExpr_backend(const volatile VecTVecMapExpr<MT> *);

FalseType isVecTVecMapExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is an outer map expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is an outer map expression
// template (i.e. an expression representing a custom operation between a column vector and a row
// vector). In order to qualify as a valid outer map expression template, the given type has to
// derive publicly from the VecTVecMapExpr base class. In case the given type is a valid outer
// map expression template, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsVecTVecMapExpr : public decltype(isVecTVecMapExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVecTVecMapExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsVecTVecMapExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVecTVecMapExpr type trait.
// \ingroup math_type_traits
//
// The IsVecTVecMapExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsVecTVecMapExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVecTVecMapExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVecTVecMapExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVecTVecMapExpr_v = IsVecTVecMapExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
