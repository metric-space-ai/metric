// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATEXPEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATEXPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatExpExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatExpExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isMatExpExpr_backend(const volatile MatExpExpr<MT> *);

FalseType isMatExpExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a matrix exponential expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a matrix exponential
// expression template. In order to qualify as a valid matrix exponential expression template,
// the given type has to derive publicly from the MatExpExpr base class. In case the given
// type is a valid matrix exponential expression template, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsMatExpExpr : public decltype(isMatExpExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatExpExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatExpExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatExpExpr type trait.
// \ingroup math_type_traits
//
// The IsMatExpExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatExpExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatExpExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatExpExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatExpExpr_v = IsMatExpExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
