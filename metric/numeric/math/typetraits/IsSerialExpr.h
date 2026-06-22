// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSERIALEXPR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSERIALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SerialExpr.h>
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
/*!\brief Auxiliary helper functions for the IsSerialExpr type trait.
// \ingroup math_type_traits
*/
template <typename U> TrueType isSerialExpr_backend(const volatile SerialExpr<U> *);

FalseType isSerialExpr_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a serial evaluation expression template.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a serial evaluation
// expression template. In order to qualify as a valid serial evaluation expression template,
// the given type has to derive publicly from the SerialExpr base class. In case the given type
// is a valid serial evaluation expression template, the \a value member constant is set to
// \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.
*/
template <typename T> struct IsSerialExpr : public decltype(isSerialExpr_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSerialExpr type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsSerialExpr<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSerialExpr type trait.
// \ingroup math_type_traits
//
// The IsSerialExpr_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSerialExpr class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSerialExpr<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSerialExpr_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSerialExpr_v = IsSerialExpr<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
