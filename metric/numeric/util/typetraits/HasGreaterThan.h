// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_HASGREATERTHAN_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_HASGREATERTHAN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsDetected.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Auxiliary type alias for the HasGreaterThan type trait.
// \ingroup type_traits
*/
template <typename T1, typename T2> using GreaterThan_t = decltype(std::declval<T1>() > std::declval<T2>());
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for the availability of a greater-than operation between two data types.
// \ingroup type_traits
//
// This type trait determines whether the given data types \a T1 and \a T2 can be used in a
// greater-than operation. If the operation is available, the \a value member constant is set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   mtrc::numeric::HasGreaterThan< int, int >::value                       // Evaluates to 1
   mtrc::numeric::HasGreaterThan< const std::string, std::string >::Type  // Results in TrueType
   mtrc::numeric::HasGreaterThan< volatile int*, int* >                   // Is derived from TrueType
   mtrc::numeric::HasGreaterThan< int, mtrc::numeric::complex<float> >::value     // Evaluates to 0
   mtrc::numeric::HasGreaterThan< std::string, int >::Type                // Results in FalseType
   mtrc::numeric::HasGreaterThan< int*, std::string* >                    // Is derived from FalseType
   \endcode
*/
template <typename T1, typename T2> using HasGreaterThan = IsDetected<GreaterThan_t, T1, T2>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasGreaterThan type trait.
// \ingroup type_traits
//
// The HasGreaterThan_v variable template provides a convenient shortcut to access the nested
// \a value of the HasGreaterThan class template. For instance, given the types \a T1 and \a T2
// the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasGreaterThan<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HasGreaterThan_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> constexpr bool HasGreaterThan_v = HasGreaterThan<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
