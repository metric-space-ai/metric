// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATEVALEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATEVALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatEvalExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatEvalExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isMatEvalExpr_backend(const volatile MatEvalExpr<MT> *);

FalseType isMatEvalExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a matrix evaluation expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a matrix evaluation
// expression template. In order to qualify as a valid matrix evaluation expression template,
// the given type has to derive publicly from the MatEvalExpr base class. In case the given
// type is a valid matrix evaluation expression template, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsMatEvalExpr : public decltype(isMatEvalExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatEvalExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatEvalExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatEvalExpr type trait.
// \ingroup math_type_traits
//
// The IsMatEvalExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatEvalExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatEvalExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatEvalExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatEvalExpr_v = IsMatEvalExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
