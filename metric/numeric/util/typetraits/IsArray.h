// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISARRAY_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISARRAY_H
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
/*!\brief Compile time type check.
// \ingroup type_traits
//
// The IsArray type trait tests whether or not the given template parameter is an array type. In
// case the given data type is an array type, the \a value member constant is set to \a true, the
// nested type definition \a Type is set to \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType and the class derives from
// \a FalseType.

   \code
   mtrc::numeric::IsArray< int[3] >::value      // Evaluates to 'true'
   mtrc::numeric::IsArray< const int[] >::Type  // Results in TrueType
   mtrc::numeric::IsArray< int[][3] >           // Is derived from TrueType
   mtrc::numeric::IsArray< int >::value         // Evaluates to 'false'
   mtrc::numeric::IsArray< int const* >::Type   // Results in FalseType
   mtrc::numeric::IsArray< std::vector<int> >   // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsArray : public BoolConstant<std::is_array<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsArray type trait.
// \ingroup type_traits
//
// The IsArray_v variable template provides a convenient shortcut to access the nested \a value
// of the IsArray class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsArray<T>::value;
   constexpr bool value2 = mtrc::numeric::IsArray_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsArray_v = IsArray<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
