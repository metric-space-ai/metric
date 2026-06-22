// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISVECSCALARDIVEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISVECSCALARDIVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/VecScalarDivExpr.h>
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
/*!\brief Auxiliary helper functions for the IsVecScalarDivExpr type trait.
// \ingroup math_type_traits
*/
template <typename VT> TrueType isVecScalarDivExpr_backend(const volatile VecScalarDivExpr<VT> *);

FalseType isVecScalarDivExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a vector/scalar division expression
//        template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a vector/scalar division
// expression template. In order to qualify as a valid vector/scalar division expression template,
// the given type has to derive publicly from the VecScalarDivExpr base class. In case the given
// type is a valid vector/scalar division expression template, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsVecScalarDivExpr : public decltype(isVecScalarDivExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVecScalarDivExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsVecScalarDivExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVecScalarDivExpr type trait.
// \ingroup math_type_traits
//
// The IsVecScalarDivExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsVecScalarDivExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVecScalarDivExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVecScalarDivExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVecScalarDivExpr_v = IsVecScalarDivExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
