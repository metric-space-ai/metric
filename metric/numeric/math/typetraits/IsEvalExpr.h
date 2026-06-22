// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISEVALEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISEVALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/EvalExpr.h>
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
/*!\brief Auxiliary helper functions for the IsEvalExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isEvalExpr_backend(const volatile EvalExpr<U> *);

FalseType isEvalExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is an evaluation expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is an evaluation expression
// template. In order to qualify as a valid evaluation expression template, the given type has
// to derive publicly from the EvalExpr base class. In case the given type is a valid evaluation
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsEvalExpr : public decltype(isEvalExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsEvalExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsEvalExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsEvalExpr type trait.
// \ingroup math_type_traits
//
// The IsEvalExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsEvalExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsEvalExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsEvalExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsEvalExpr_v = IsEvalExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
