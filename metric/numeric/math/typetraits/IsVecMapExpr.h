// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISVECMAPEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISVECMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/VecMapExpr.h>
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
/*!\brief Auxiliary helper functions for the IsVecMapExpr type trait.
// \ingroup math_type_traits
*/
template <typename VT> TrueType isVecMapExpr_backend(const volatile VecMapExpr<VT> *);

FalseType isVecMapExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a unary vector map expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a unary vector map
// expression template. In order to qualify as a valid unary vector map expression template,
// the given type has to derive publicly from the VecMapExpr base class. In case the given
// type is a valid unary vector map expression template, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsVecMapExpr : public decltype(isVecMapExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVecMapExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsVecMapExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVecMapExpr type trait.
// \ingroup math_type_traits
//
// The IsVecMapExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsVecMapExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVecMapExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVecMapExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVecMapExpr_v = IsVecMapExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
