// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISEXPRESSION_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISEXPRESSION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Expression.h>
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
/*!\brief Auxiliary helper functions for the IsExpression type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isExpression_backend(const volatile Expression<U> *);

FalseType isExpression_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is an expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a Metric numeric expression
// template. In order to qualify as a valid expression template, the given type has to derive
// publicly from the Expression base class. In case the given type is a valid expression
// template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is
// set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsExpression : public decltype(isExpression_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsExpression type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsExpression<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsExpression type trait.
// \ingroup math_type_traits
//
// The IsExpression_v variable template provides a convenient shortcut to access the nested
// \a value of the IsExpression class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsExpression<T>::value;
   constexpr bool value2 = mtrc::numeric::IsExpression_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsExpression_v = IsExpression<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
