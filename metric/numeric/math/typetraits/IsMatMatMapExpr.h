// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATMATMAPEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATMATMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatMatMapExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatMatMapExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isMatMatMapExpr_backend(const volatile MatMatMapExpr<MT> *);

FalseType isMatMatMapExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a binary matrix map expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a binary matrix map
// expression template. In order to qualify as a valid binary matrix map expression template,
// the given type has to derive publicly from the MatMatMapExpr base class. In case the given
// type is a valid binary matrix map expression template, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsMatMatMapExpr : public decltype(isMatMatMapExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatMatMapExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatMatMapExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatMatMapExpr type trait.
// \ingroup math_type_traits
//
// The IsMatMatMapExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatMatMapExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatMatMapExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatMatMapExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatMatMapExpr_v = IsMatMatMapExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
