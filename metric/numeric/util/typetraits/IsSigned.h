// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISSIGNED_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISSIGNED_H
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
/*!\brief Compile time check for signed data types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a signed integral
// or a floating point data type. In case the type is a signed (possibly cv-qualified) data
// type, the \a value member constant is set to \a true, the nested type definition \a Type
// is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   class MyClass {};

   mtrc::numeric::IsSigned<short>::value               // Evaluates to 'true'
   mtrc::numeric::IsSigned<const int>::Type            // Results in TrueType
   mtrc::numeric::IsSigned<volatile float>             // Is derived from TrueType
   mtrc::numeric::IsSigned<unsigned int>::value        // Evaluates to 'false'
   mtrc::numeric::IsSigned<const unsigned long>::Type  // Results in FalseType
   mtrc::numeric::IsSigned<MyClass>                    // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsSigned : public BoolConstant<std::is_signed<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSigned type trait.
// \ingroup type_traits
//
// The IsSigned_v variable template provides a convenient shortcut to access the nested \a value
// of the IsSigned class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSigned<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSigned_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSigned_v = IsSigned<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
