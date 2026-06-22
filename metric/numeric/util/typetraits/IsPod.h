// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISPOD_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISPOD_H
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
/*!\brief Compile time check for pod data types.
// \ingroup type_traits
//
// This type trait tests whether or not the given template parameter is a POD (Plain Old Data).
// In case the type is a POD, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   class A {
	  int i_;
	  double d_;
   };

   class B {
	  virtual ~B() {}
   };

   class C {
	  std::string s_;
   };

   mtrc::numeric::IsPod<int>::value                 // Evaluates to 'true'
   mtrc::numeric::IsPod<double const>::Type         // Results in TrueType
   mtrc::numeric::IsPod<A volatile>                 // Is derived from TrueType
   mtrc::numeric::IsPod< std::vector<int> >::value  // Evaluates to 'false'
   mtrc::numeric::IsPod<B>::Type                    // Results in FalseType
   mtrc::numeric::IsPod<C>                          // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsPod : public BoolConstant<std::is_pod<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsPod type trait.
// \ingroup type_traits
//
// The IsPod_v variable template provides a convenient shortcut to access the nested \a value of
// the IsPod class template. For instance, given the type \a T the following two statements are
// identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsPod<T>::value;
   constexpr bool value2 = mtrc::numeric::IsPod_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsPod_v = IsPod<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
