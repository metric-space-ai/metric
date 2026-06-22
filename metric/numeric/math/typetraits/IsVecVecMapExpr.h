// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISVECVECMAPEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISVECVECMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/VecVecMapExpr.h>
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
/*!\brief Auxiliary helper functions for the IsVecVecMapExpr type trait.
// \ingroup math_type_traits
*/
template <typename VT> TrueType isVecVecMapExpr_backend(const volatile VecVecMapExpr<VT> *);

FalseType isVecVecMapExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a binary vector map expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a binary vector map
// expression template. In order to qualify as a valid binary vector map expression template,
// the given type has to derive publicly from the VecVecMapExpr base class. In case the given
// type is a valid binary vector map expression template, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsVecVecMapExpr : public decltype(isVecVecMapExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVecVecMapExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsVecVecMapExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVecVecMapExpr type trait.
// \ingroup math_type_traits
//
// The IsVecVecMapExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsVecVecMapExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVecVecMapExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVecVecMapExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVecVecMapExpr_v = IsVecVecMapExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
