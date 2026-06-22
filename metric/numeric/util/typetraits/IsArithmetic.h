// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISARITHMETIC_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISARITHMETIC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <type_traits>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for arithmetic data types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a (possibly cv-qualified)
// arithmetic (integral or floating point) data type. In case the type is an arithmetic type,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType and the class derives from \a FalseType.

   \code
   class MyClass {};

   mtrc::numeric::IsArithmetic<int>::value         // Evaluates to 'true'
   mtrc::numeric::IsArithmetic<float const>::Type  // Results in TrueType
   mtrc::numeric::IsArithmetic<short volatile>     // Is derived from TrueType
   mtrc::numeric::IsArithmetic<void>::value        // Evaluates to 'false'
   mtrc::numeric::IsArithmetic<int*>::value        // Evaluates to 'false'
   mtrc::numeric::IsArithmetic<int&>::Type         // Results in FalseType
   mtrc::numeric::IsArithmetic<MyClass>            // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsArithmetic : public BoolConstant<std::is_arithmetic<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsArithmetic type trait.
// \ingroup type_traits
//
// The IsArithmetic_v variable template provides a convenient shortcut to access the nested
// \a value of the IsArithmetic class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsArithmetic<T>::value;
   constexpr bool value2 = mtrc::numeric::IsArithmetic_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsArithmetic_v = IsArithmetic<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
