// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPELIST_TYPELIST_H
#define METRIC_NUMERIC_UTIL_TYPELIST_TYPELIST_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS TYPELIST
//
//=================================================================================================

//*************************************************************************************************
/*!\defgroup typelist Type lists
// \ingroup util
//
// Type lists provide the functionality to create lists of data types. In constrast to lists of
// data values (as for instance the std::list class template), type lists are created at compile
// time, not at run time. The following example demonstrates, how type lists are created and
// manipulated:

   \code
   // Creating a type list consisting of two fundamental floating point data types
   using Tmp = mtrc::numeric::TypeList< float, double >;

   // Appending a type to the type list
   using Floats = mtrc::numeric::Append< Tmp, long double >::Type;  // Type list contains all floating point data
types

   // Calculating the length of the type list (at compile time!)
   const int length = mtrc::numeric::Length< Floats >::value;  // Value evaluates to 3

   // Accessing a specific type of the type list via indexing
   using Index0 = mtrc::numeric::TypeAt< Floats, 0 >::Type;

   // Searching the type list for a specific type
   constexpr bool index1 = mtrc::numeric::Contains< Floats, double >::value;  // Value evaluates to 1
   constexpr bool index2 = mtrc::numeric::Contains< Floats, int    >::value;  // Value evaluates to 0

   // Estimating the index of a specific type in the type list
   constexpr bool index3 = mtrc::numeric::IndexOf< Floats, double >::value;   // Value evaluates to 1
   constexpr bool index4 = mtrc::numeric::IndexOf< Floats, int    >::value;   // Value evaluates to 3

   // Erasing the first occurrence of float from the type list
   using NoFloat = mtrc::numeric::Erase< Floats, float >::Type;

   // Removing all duplicates from the type list
   using NoDuplicates = mtrc::numeric::Unique< Floats >::Type;
   \endcode
*/
/*!\brief Implementation of a type list.
// \ingroup typelist
//
// The TypeList class template represents a list of data types of arbitrary size. The following
// example gives an impression how type lists are used and manipulated:

   \code
   // Creating a type list consisting of two fundamental floating point data types
   using Tmp = mtrc::numeric::TypeList< float, double >;

   // Appending a type to the type list
   using Floats = mtrc::numeric::Append< Tmp, long double >::Type;  // Type list contains all floating point data
types

   // Calculating the length of the type list (at compile time!)
   const int length = mtrc::numeric::Length< Floats >::value;  // Value evaluates to 3

   // Accessing a specific type of the type list via indexing
   using Index0 = mtrc::numeric::TypeAt< Floats, 0 >::Type;

   // Searching the type list for a specific type
   constexpr bool index1 = mtrc::numeric::Contains< Floats, double >::value;  // Value evaluates to 1
   constexpr bool index2 = mtrc::numeric::Contains< Floats, int    >::value;  // Value evaluates to 0

   // Estimating the index of a specific type in the type list
   constexpr bool index3 = mtrc::numeric::IndexOf< Floats, double >::value;   // Value evaluates to 1
   constexpr bool index4 = mtrc::numeric::IndexOf< Floats, int    >::value;   // Value evaluates to 3

   // Erasing the first occurrence of float from the type list
   using NoFloat = mtrc::numeric::Erase< Floats, float >::Type;

   // Removing all duplicates from the type list
   using NoDuplicates = mtrc::numeric::Unique< Floats >::Type;
   \endcode
*/
template <typename... Ts> struct TypeList {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
