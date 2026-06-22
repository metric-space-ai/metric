// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISBINARYMAPEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISBINARYMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/BinaryMapExpr.h>
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
/*!\brief Auxiliary helper functions for the IsBinaryMapExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isBinaryMapExpr_backend(const volatile BinaryMapExpr<U> *);

FalseType isBinaryMapExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a binary map expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a binary map expression
// template. In order to qualify as a valid binary map expression template, the given type has
// to derive publicly from the BinaryMapExpr base class. In case the given type is a valid binary
// map expression template, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsBinaryMapExpr : public decltype(isBinaryMapExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsBinaryMapExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsBinaryMapExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsBinaryMapExpr type trait.
// \ingroup math_type_traits
//
// The IsBinaryMapExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsBinaryMapExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsBinaryMapExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsBinaryMapExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsBinaryMapExpr_v = IsBinaryMapExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
