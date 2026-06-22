// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATSERIALEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATSERIALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/MatSerialExpr.h>
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
/*!\brief Auxiliary helper functions for the IsMatSerialExpr type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isMatSerialExpr_backend(const volatile MatSerialExpr<MT> *);

FalseType isMatSerialExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a matrix serial evaluation expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a matrix serial evaluation
// expression template. In order to qualify as a valid matrix serial evaluation expression template,
// the given type has to derive publicly from the MatSerialExpr base class. In case the given type
// is a valid matrix serial evaluation expression template, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsMatSerialExpr : public decltype(isMatSerialExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatSerialExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatSerialExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatSerialExpr type trait.
// \ingroup math_type_traits
//
// The IsMatSerialExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsMatSerialExpr class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatSerialExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatSerialExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatSerialExpr_v = IsMatSerialExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
