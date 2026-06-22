// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_REQUIRESEVALUATION_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_REQUIRESEVALUATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check to query the requirement to evaluate an expression.
// \ingroup math_type_traits
//
// Via this type trait it is possible to determine whether a given vector or matrix expression
// type requires an intermediate evaluation in the context of a compound expression. In case
// the given type requires an evaluation, the \a value member constant is set to \a true, the
// nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives
// from \a FalseType.
//
// \note that this type trait can only be applied to Metric numeric vector or matrix expressions
// or any other type providing the nested type \a CompositeType. In case this nested type
// is not available, applying the type trait results in a compile time error!
*/
template <typename T> struct RequiresEvaluation : public BoolConstant<!IsReference_v<typename T::CompositeType>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the RequiresEvaluation type trait.
// \ingroup math_type_traits
//
// The RequiresEvaluation_v variable template provides a convenient shortcut to access the nested
// \a value of the RequiresEvaluation class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::RequiresEvaluation<T>::value;
   constexpr bool value2 = mtrc::numeric::RequiresEvaluation_v<T>;
   \endcode
*/
template <typename T> constexpr bool RequiresEvaluation_v = RequiresEvaluation<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
