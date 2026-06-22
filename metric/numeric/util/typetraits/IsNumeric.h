// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISNUMERIC_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISNUMERIC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsArithmetic.h>
#include <metric/numeric/util/typetraits/IsBoolean.h>
#include <metric/numeric/util/typetraits/IsComplex.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for numeric types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a numeric data type.
// Metric numeric considers all integral (except \a bool), floating point, and complex data types as
// numeric data types. In case the type is a numeric type, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives
// from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the
// class derives from \a FalseType.

   \code
   mtrc::numeric::IsNumeric<int>::value                // Evaluates to 'true' (int is a numeric data type)
   mtrc::numeric::IsNumeric<const double>::Type        // Results in TrueType (double is a numeric data type)
   mtrc::numeric::IsNumeric<volatile complex<float> >  // Is derived from TrueType (complex<float> is a numeric data
type) mtrc::numeric::IsNumeric<void>::value               // Evaluates to 'false' (void is not a numeric data type)
   mtrc::numeric::IsNumeric<bool>::Type                // Results in FalseType (bool is not a numeric data type)
   mtrc::numeric::IsNumeric<const bool>                // Is derived from FalseType
   \endcode
*/
template <typename T>
struct IsNumeric : public BoolConstant<(IsArithmetic_v<T> && !IsBoolean_v<T>) || IsComplex_v<T>> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsNumeric type trait for 'const' qualified types.
template <typename T> struct IsNumeric<const T> : public IsNumeric<T>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsNumeric type trait for 'volatile' qualified types.
template <typename T> struct IsNumeric<volatile T> : public IsNumeric<T>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the IsNumeric type trait for 'const volatile' qualified types.
template <typename T> struct IsNumeric<const volatile T> : public IsNumeric<T>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsNumeric type trait.
// \ingroup type_traits
//
// The IsNumeric_v variable template provides a convenient shortcut to access the nested
// \a value of the IsNumeric class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsNumeric<T>::value;
   constexpr bool value2 = mtrc::numeric::IsNumeric_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsNumeric_v = IsNumeric<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
