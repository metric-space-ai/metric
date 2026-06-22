// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATVECSOLVEEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATVECSOLVEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatVecSolveExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatVecSolveExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isMatVecSolveExpr_backend(const volatile MatVecSolveExpr<MT> *);

FalseType isMatVecSolveExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a single LSE solver expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a single LSE solver
// expression template (i.e. a solver expression for a single right-hand side vector). In order
// to qualify as a single LSE solver expression template, the given type has to derive publicly
// from the MatVecSolveExpr base class. In case the given type is a valid single LSE solver
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsMatVecSolveExpr : public decltype(isMatVecSolveExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatVecSolveExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatVecSolveExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatVecSolveExpr type trait.
// \ingroup math_type_traits
//
// The IsMatVecSolveExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatVecSolveExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatVecSolveExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatVecSolveExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatVecSolveExpr_v = IsMatVecSolveExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
