// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATMATSOLVEEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATMATSOLVEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatMatSolveExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatMatSolveExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isMatMatSolveExpr_backend(const volatile MatMatSolveExpr<MT> *);

FalseType isMatMatSolveExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a multi LSE solver expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a multi LSE solver
// expression template (i.e. a solver expression for multiple right-hand side vectors). In order
// to qualify as a multi LSE solver expression template, the given type has to derive publicly
// from the MatMatSolveExpr base class. In case the given type is a valid multi LSE solver
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsMatMatSolveExpr : public decltype(isMatMatSolveExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatMatSolveExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatMatSolveExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatMatSolveExpr type trait.
// \ingroup math_type_traits
//
// The IsMatMatSolveExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatMatSolveExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatMatSolveExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatMatSolveExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatMatSolveExpr_v = IsMatMatSolveExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
