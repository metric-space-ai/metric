// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISBASEOF_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISBASEOF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/RemoveCV.h>
#include <type_traits>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time analysis of an inheritance relationship.
// \ingroup type_traits
//
// This type trait tests for an inheritance relationship between the two types \a Base and
// \a Derived. If \a Derived is a type derived from \a Base or the same type as \a Base the
// \a value member contant is set to \a true, the nested type definition \a Type is \a TrueType,
// and the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.

   \code
   class A { ... };
   class B : public A { ... };
   class C { ... };

   mtrc::numeric::IsBaseOf<A,B>::value  // Evaluates to 'true'
   mtrc::numeric::IsBaseOf<A,B>::Type   // Results in TrueType
   mtrc::numeric::IsBaseOf<A,B>         // Is derived from TrueType
   mtrc::numeric::IsBaseOf<A,C>::value  // Evaluates to 'false'
   mtrc::numeric::IsBaseOf<B,A>::Type   // Results in FalseType
   mtrc::numeric::IsBaseOf<B,A>         // Is derived from FalseType
   \endcode
*/
template <typename Base, typename Derived>
class IsBaseOf : public BoolConstant<std::is_base_of<RemoveCV_t<Base>, RemoveCV_t<Derived>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsBaseOf type trait.
// \ingroup type_traits
//
// The IsBaseOf_v variable template provides a convenient shortcut to access the nested \a value
// of the IsBaseOf class template. For instance, given the types \a T1 and \a T2 the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsBaseOf<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::IsBaseOf_v<T1,T2>;
   \endcode
*/
template <typename Base, typename Derived> constexpr bool IsBaseOf_v = IsBaseOf<Base, Derived>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
