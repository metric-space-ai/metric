// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISVECGENEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISVECGENEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/VecGenExpr.h>
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
/*!\brief Auxiliary helper functions for the IsVecGenExpr type trait.
// \ingroup math_type_traits
*/
template <typename VT> TrueType isVecGenExpr_backend(const volatile VecGenExpr<VT> *);

FalseType isVecGenExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a vector generator expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a vector generator
// expression template. In order to qualify as a valid vector generator expression template,
// the given type has to derive publicly from the VecGenExpr base class. In case the given
// type is a valid vector generator expression template, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsVecGenExpr : public decltype(isVecGenExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVecGenExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsVecGenExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVecGenExpr type trait.
// \ingroup math_type_traits
//
// The IsVecGenExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsVecGenExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVecGenExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVecGenExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVecGenExpr_v = IsVecGenExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
