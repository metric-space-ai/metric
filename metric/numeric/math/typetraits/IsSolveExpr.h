// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSOLVEEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSOLVEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SolveExpr.h>
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
/*!\brief Auxiliary helper functions for the IsSolveExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isSolveExpr_backend(const volatile SolveExpr<MT> *);

FalseType isSolveExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a solver expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a solver expression
// template (i.e. an expression representing an LSE solver for a single or multiple right-hand
// side vectors). In order to qualify as a solver expression template, the given type has to
// derive publicly from the SolveExpr base class. In case the given type is a valid solver
// expression template, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsSolveExpr : public decltype(isSolveExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSolveExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsSolveExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSolveExpr type trait.
// \ingroup math_type_traits
//
// The IsSolveExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSolveExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSolveExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSolveExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSolveExpr_v = IsSolveExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
