// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISPROXY_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISPROXY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/proxy/Forward.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper functions for the IsProxy type trait.
// \ingroup math_type_traits
*/
template <typename PT, typename RT> TrueType isProxy_backend(const volatile Proxy<PT, RT> *);

FalseType isProxy_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for proxy types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a proxy type (i.e.
// publicly derived from the mtrc::numeric::Proxy class template). In case the type is a proxy,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   class MyProxy1 : public Proxy<MyProxy1> {};
   class MyProxy2 : public Proxy<MyProxy2,int> {};
   class MyProxy3 : public Proxy<MyProxy3,double> {};
   class MyProxy4 {};

   mtrc::numeric::IsProxy< MyProxy1 >::value       // Evaluates to 1
   mtrc::numeric::IsProxy< const MyProxy2 >::Type  // Results in TrueType
   mtrc::numeric::IsProxy< volatile MyProxy3 >     // Is derived from TrueType
   mtrc::numeric::IsProxy< MyProxy4 >::value       // Evaluates to 0
   mtrc::numeric::IsProxy< const MyProxy4 >::Type  // Results in FalseType
   mtrc::numeric::IsProxy< volatile MyProxy4 >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsProxy : public decltype(isProxy_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsProxy type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsProxy<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsProxy type trait.
// \ingroup math_type_traits
//
// The IsProxy_v variable template provides a convenient shortcut to access the nested \a value
// of the IsProxy class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsProxy<T>::value;
   constexpr bool value2 = mtrc::numeric::IsProxy_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsProxy_v = IsProxy<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
