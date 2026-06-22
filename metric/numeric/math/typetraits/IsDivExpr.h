// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISDIVEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISDIVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/DivExpr.h>
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
/*!\brief Auxiliary helper functions for the IsDivExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isDivExpr_backend(const volatile DivExpr<U> *);

FalseType isDivExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a division expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a division expression
// template (i.e. an expression representing a vector/scalar division or a matrix/scalar
// division). In order to qualify as a valid division expression template, the given type
// has to derive publicly from the DivExpr base class. In case the given type is a valid
// division expression template, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.
*/
template <typename T> struct IsDivExpr : public decltype(isDivExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsDivExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsDivExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDivExpr type trait.
// \ingroup math_type_traits
//
// The IsDivExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsDivExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDivExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDivExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDivExpr_v = IsDivExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
