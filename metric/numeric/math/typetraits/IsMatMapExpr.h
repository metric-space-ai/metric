// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATMAPEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatMapExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatMapExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isMatMapExpr_backend(const volatile MatMapExpr<MT> *);

FalseType isMatMapExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a unary matrix map expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a unary matrix map
// expression template. In order to qualify as a valid unary matrix map expression template,
// the given type has to derive publicly from the MatMapExpr base class. In case the given
// type is a valid unary matrix map expression template, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsMatMapExpr : public decltype(isMatMapExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatMapExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatMapExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatMapExpr type trait.
// \ingroup math_type_traits
//
// The IsMatMapExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatMapExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatMapExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatMapExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatMapExpr_v = IsMatMapExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
