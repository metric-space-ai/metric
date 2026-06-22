// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPELIST_APPEND_H
#define METRIC_NUMERIC_UTIL_TYPELIST_APPEND_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typelist/TypeList.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Appending a type to a type list.
// \ingroup typelist
//
// The Append class can be used to append the data type \a T to a type list \a TL. In order to
// append a data type, the Append class has to be instantiated for a particular type list and
// another type. The following example gives an impression of the use of the Append class:

   \code
   using Tmp    = mtrc::numeric::TypeList< float, double >;      // Defining a temporary type list
   using Floats = mtrc::numeric::Append<Tmp,long double>::Type;  // Type list contains all floating point data types
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The type to be appended to the type list
struct Append;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Append class for appending a single type.
// \ingroup typelist
*/
template <typename... Ts // Types of the type list
		  ,
		  typename T> // The type to be appended to the type list
struct Append<TypeList<Ts...>, T> {
	using Type = TypeList<Ts..., T>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Append class for appending a type list.
// \ingroup typelist
*/
template <typename... Ts1 // Type of the type list
		  ,
		  typename... Ts2> // The types to be appended to the type list
struct Append<TypeList<Ts1...>, TypeList<Ts2...>> {
	using Type = TypeList<Ts1..., Ts2...>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the Append class template.
// \ingroup type_traits
//
// The Append_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the Append class template. For instance, given the type list \a TL and the type \a T the
// following two type definitions are identical:

   \code
   using Type1 = typename Append<TL,T>::Type;
   using Type2 = Append_t<TL,T>;
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The type to be appended to the type list
using Append_t = typename Append<TL, T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
