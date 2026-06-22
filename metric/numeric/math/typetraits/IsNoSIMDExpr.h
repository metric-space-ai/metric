// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISNOSIMDEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISNOSIMDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/NoSIMDExpr.h>
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
/*!\brief Auxiliary helper functions for the IsNoSIMDExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isNoSIMDExpr_backend(const volatile NoSIMDExpr<U> *);

FalseType isNoSIMDExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a no-SIMD expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a no-SIMD expression
// template. In order to qualify as a valid no-SIMD expression template, the given type has
// to derive publicly from the NoSIMDExpr base class. In case the given type is a valid no-SIMD
// expression template, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsNoSIMDExpr : public decltype(isNoSIMDExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsNoSIMDExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsNoSIMDExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNoSIMDExpr type trait.
// \ingroup math_type_traits
//
// The IsNoSIMDExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsNoSIMDExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNoSIMDExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsNoSIMDExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsNoSIMDExpr_v = IsNoSIMDExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
