// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISDECLUNIUPPEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISDECLUNIUPPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/DeclUniUppExpr.h>
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
/*!\brief Auxiliary helper functions for the IsDeclUniUppExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isDeclUniUppExpr_backend(const volatile DeclUniUppExpr<MT> *);

FalseType isDeclUniUppExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a decluniupp expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a decluniupp expression
// template. In order to qualify as a valid decluniupp expression template, the given type has
// to derive publicly from the DeclUniUppExpr base class. In case the given type is a valid
// decluniupp expression template, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T> struct IsDeclUniUppExpr : public decltype(isDeclUniUppExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsDeclUniUppExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsDeclUniUppExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDeclUniUppExpr type trait.
// \ingroup math_type_traits
//
// The IsDeclUniUppExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsDeclUniUppExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDeclUniUppExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDeclUniUppExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDeclUniUppExpr_v = IsDeclUniUppExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
