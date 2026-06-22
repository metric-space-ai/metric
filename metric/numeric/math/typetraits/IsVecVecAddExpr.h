// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISVECVECADDEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISVECVECADDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/VecVecAddExpr.h>
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
/*!\brief Auxiliary helper functions for the IsVecVecAddExpr type trait.
// \ingroup math_type_traits
*/
template <typename VT> TrueType isVecVecAddExpr_backend(const volatile VecVecAddExpr<VT> *);

FalseType isVecVecAddExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a vector/vector addition expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a vector/vector addition
// expression template. In order to qualify as a valid vector addition expression template, the
// given type has to derive publicly from the VecVecAddExpr base class. In case the given type is
// a valid vector addition expression template, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.
*/
template <typename T> struct IsVecVecAddExpr : public decltype(isVecVecAddExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVecVecAddExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsVecVecAddExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVecVecAddExpr type trait.
// \ingroup math_type_traits
//
// The IsVecVecAddExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsVecVecAddExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVecVecAddExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVecVecAddExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVecVecAddExpr_v = IsVecVecAddExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
