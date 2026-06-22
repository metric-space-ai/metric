// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISCONVERTIBLE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISCONVERTIBLE_H
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
/*!\brief Compile time pointer relationship constraint.
// \ingroup type_traits
//
// This type traits tests whether the first given template argument can be converted to the
// second template argument via copy construction. If the first argument can be converted
// to the second argument, the \a value member constnt is set to \a true, the nested type
// definition \a type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   struct A {};
   struct B : public A {};

   struct C {};
   struct D {
	  D( const C& c ) {}
   };

   mtrc::numeric::IsConvertible<int,unsigned int>::value    // Evaluates to 'true'
   mtrc::numeric::IsConvertible<float,const double>::value  // Evaluates to 'true'
   mtrc::numeric::IsConvertible<B,A>::Type                  // Results in TrueType
   mtrc::numeric::IsConvertible<B*,A*>::Type                // Results in TrueType
   mtrc::numeric::IsConvertible<C,D>                        // Is derived from TrueType
   mtrc::numeric::IsConvertible<char*,std::string>          // Is derived from TrueType
   mtrc::numeric::IsConvertible<std::string,char*>::value   // Evaluates to 'false'
   mtrc::numeric::IsConvertible<A,B>::Type                  // Results in FalseType
   mtrc::numeric::IsConvertible<A*,B*>                      // Is derived from FalseType
   \endcode
*/
template <typename From, typename To>
struct IsConvertible : public BoolConstant<std::is_convertible<From, To>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsConvertible type trait.
// \ingroup type_traits
//
// The IsConvertible_v variable template provides a convenient shortcut to access the nested
// \a value of the IsConvertible class template. For instance, given the types \a T1 and \a T2
// the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsConvertible<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::IsConvertible_v<T1,T2>;
   \endcode
*/
template <typename From, typename To> constexpr bool IsConvertible_v = IsConvertible<From, To>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
