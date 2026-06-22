// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISUNSIGNED_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISUNSIGNED_H
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
/*!\brief Compile time check for unsigned data types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is an unsigned, integral
// data type. In case the type is an unsigned (possibly cv-qualified) data type, the \a value
// member constant is set to \a true, the nested type definition \a Type is \a TrueType, and
// the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.

   \code
   class MyClass {};

   mtrc::numeric::IsUnsigned<unsigned short>::value     // Evaluates to 'true'
   mtrc::numeric::IsUnsigned<const unsigned int>::Type  // Results in TrueType
   mtrc::numeric::IsUnsigned<volatile unsigned long>    // Is derived from TrueType
   mtrc::numeric::IsUnsigned<float>::value              // Evaluates to 'false'
   mtrc::numeric::IsUnsigned<const volatile int>::Type  // Results in FalseType
   mtrc::numeric::IsUnsigned<MyClass>                   // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsUnsigned : public BoolConstant<std::is_unsigned<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsUnsigned type trait.
// \ingroup type_traits
//
// The IsUnsigned_v variable template provides a convenient shortcut to access the nested
// \a value of the IsUnsigned class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsUnsigned<T>::value;
   constexpr bool value2 = mtrc::numeric::IsUnsigned_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsUnsigned_v = IsUnsigned<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
