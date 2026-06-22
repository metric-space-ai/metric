// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISVECNOSIMDEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISVECNOSIMDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/VecNoSIMDExpr.h>
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
/*!\brief Auxiliary helper functions for the IsVecNoSIMDExpr type trait.
// \ingroup math_type_traits
*/
template <typename VT> TrueType isVecNoSIMDExpr_backend(const volatile VecNoSIMDExpr<VT> *);

FalseType isVecNoSIMDExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a vector no-SIMD expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a vector no-SIMD
// expression template. In order to qualify as a valid vector no-SIMD expression template,
// the given type has to derive publicly from the VecNoSIMDExpr base class. In case the given
// type is a valid vector no-SIMD expression template, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsVecNoSIMDExpr : public decltype(isVecNoSIMDExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVecNoSIMDExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsVecNoSIMDExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVecNoSIMDExpr type trait.
// \ingroup math_type_traits
//
// The IsVecNoSIMDExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsVecNoSIMDExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVecNoSIMDExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVecNoSIMDExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVecNoSIMDExpr_v = IsVecNoSIMDExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
