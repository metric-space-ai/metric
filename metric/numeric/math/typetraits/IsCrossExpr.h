// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCROSSEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCROSSEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/CrossExpr.h>
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
/*!\brief Auxiliary helper functions for the IsCrossExpr type trait.
// \ingroup math_type_traits
*/
template <typename VT> TrueType isCrossExpr_backend(const volatile CrossExpr<VT> *);

FalseType isCrossExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a cross product expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a cross product
// expression template. In order to qualify as a valid cross product expression template, the
// given type has to derive publicly from the CrossExpr base class. In case the given type is
// a valid cross product expression template, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.
*/
template <typename T> struct IsCrossExpr : public decltype(isCrossExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsCrossExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsCrossExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsCrossExpr type trait.
// \ingroup math_type_traits
//
// The IsCrossExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsCrossExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsCrossExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsCrossExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsCrossExpr_v = IsCrossExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
