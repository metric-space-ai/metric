// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISVECSCALARMULTEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISVECSCALARMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/VecScalarMultExpr.h>
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
/*!\brief Auxiliary helper functions for the IsVecScalarMultExpr type trait.
// \ingroup math_type_traits
*/
template <typename VT> TrueType isVecScalarMultExpr_backend(const volatile VecScalarMultExpr<VT> *);

FalseType isVecScalarMultExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a vector/scalar multiplication expression
//        template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a vector/scalar
// multiplication expression template. In order to qualify as a valid vector/scalar
// multiplication expression template, the given type has to derive publicly from the
// VecScalarMultExpr base class. In case the given type is a valid vector/scalar multiplication
// expression template, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.
*/
template <typename T> struct IsVecScalarMultExpr : public decltype(isVecScalarMultExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVecScalarMultExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsVecScalarMultExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVecScalarMultExpr type trait.
// \ingroup math_type_traits
//
// The IsVecScalarMultExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsVecScalarMultExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVecScalarMultExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVecScalarMultExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVecScalarMultExpr_v = IsVecScalarMultExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
