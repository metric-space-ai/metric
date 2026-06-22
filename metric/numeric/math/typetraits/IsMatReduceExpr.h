// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATREDUCEEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATREDUCEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatReduceExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatReduceExpr type trait.
// \ingroup math_type_traits
*/
template <typename U, ReductionFlag N> TrueType isMatReduceExpr_backend(const volatile MatReduceExpr<U, N> *);

FalseType isMatReduceExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a reduction expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a matrix reduction
// expression template. In order to qualify as a valid matrix reduction expression template, the
// given type has to derive publicly from the MatReduceExpr base class. In case the given type
// is a valid reduction expression template, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.
*/
template <typename T> struct IsMatReduceExpr : public decltype(isMatReduceExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatReduceExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatReduceExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatReduceExpr type trait.
// \ingroup math_type_traits
//
// The IsMatReduceExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatReduceExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatReduceExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatReduceExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatReduceExpr_v = IsMatReduceExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
