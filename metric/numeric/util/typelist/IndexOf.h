// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPELIST_INDEXOF_H
#define METRIC_NUMERIC_UTIL_TYPELIST_INDEXOF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typelist/TypeList.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  TYPE LIST SEARCH
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Searching a type list.
// \ingroup typelist
//
// The IndexOf class can be used to search the type list for a particular type \a Type. In
// contrast to the Contains and the ContainsRelated classes, the IndexOf class evaluates the
// index of the given type in the type list. In case the type is contained in the type list,
// the \a value member represents the index of the queried type. Otherwise the \a value member
// is set to the length of the type list. In order to search for a type, the IndexOf class has
// to be instantiated for a particular type list and a search type. The following example gives
// an impression of the use of the IndexOf class:

   \code
   using Floats = mtrc::numeric::TypeList< float, double, long double >;     // Defining a new type list
   constexpr bool index1 = mtrc::numeric::IndexOf< Floats, double >::value;  // Value evaluates to 1
   constexpr bool index2 = mtrc::numeric::IndexOf< Floats, int    >::value;  // Value evaluates to -1
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The search type
struct IndexOf;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the IndexOf class for empty type lists.
// \ingroup typelist
*/
template <typename T> // The search type
struct IndexOf<TypeList<>, T> : public Size_t<1UL> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the IndexOf class for a successful search.
// \ingroup typelist
*/
template <typename T // The search type
		  ,
		  typename... Ts> // Types of the tail of the type list
struct IndexOf<TypeList<T, Ts...>, T> : public Size_t<0UL> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the IndexOf class for a general type list.
// \ingroup typelist
*/
template <typename U // Type of the head of the type list
		  ,
		  typename... Ts // Types of the tail of the type list
		  ,
		  typename T> // The search type
struct IndexOf<TypeList<U, Ts...>, T> : public Size_t<1UL + IndexOf<TypeList<Ts...>, T>::value> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IndexOf type trait.
// \ingroup type_traits
//
// The IndexOf_v variable template provides a convenient shortcut to access the nested \a value
// of the IndexOf class template. For instance, given the type list \a TL and the type \a T the
// following two statements are identical:

   \code
   constexpr size_t value1 = IndexOf<TL,T>::value;
   constexpr size_t value2 = IndexOf_v<TL,T>;
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The search type
constexpr size_t IndexOf_v = IndexOf<TL, T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
