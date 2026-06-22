// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMULTEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MultExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMultExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isMultExpr_backend(const volatile MultExpr<U> *);

FalseType isMultExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a multiplication expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a multiplication
// expression template (i.e. an expression representing an element-wise vector multiplication,
// a matrix/vector multiplication, a vector/matrix multiplication, or a matrix multiplication).
// In order to qualify as a valid multiplication expression template, the given type has to
// derive publicly from the MultExpr base class. In case the given type is a valid multiplication
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsMultExpr : public decltype(isMultExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMultExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMultExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMultExpr type trait.
// \ingroup math_type_traits
//
// The IsMultExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMultExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMultExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMultExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMultExpr_v = IsMultExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
