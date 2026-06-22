// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPELIST_ERASE_H
#define METRIC_NUMERIC_UTIL_TYPELIST_ERASE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typelist/Append.h>
#include <metric/numeric/util/typelist/TypeList.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Erasing the first occurrence of a type from a type list.
// \ingroup typelist
//
// The Erase class can be used to erase the first occurrence of data type \a T from a type
// list \a TL. In order to erase the first occurrence of a data type, the Erase class has to
// be instantiated for a particular type list and another type. The following example gives
// an impression of the use of the Erase class:

   \code
   // Defining a temporary type list containing the type int twice
   using Tmp = mtrc::numeric::TypeList< float, int, double, int >;

   // Erasing the first occurrence of int from the type list
   using SingleInt = mtrc::numeric::Erase<Tmp,int>::Type;
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The type to be erased from the type list
struct Erase;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Erase class for empty type lists.
// \ingroup typelist
*/
template <typename T> // The type to be erased from the type list
struct Erase<TypeList<>, T> {
	using Type = TypeList<>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Erase class for erasing the first occurrence of T.
// \ingroup typelist
*/
template <typename T // The type to be erased from the type list
		  ,
		  typename... Ts> // Types of the tail of the type list
struct Erase<TypeList<T, Ts...>, T> {
	using Type = TypeList<Ts...>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Erase class for a general type list.
// \ingroup typelist
*/
template <typename U // Type of the head of the type list
		  ,
		  typename... Ts // Types of the tail of the type list
		  ,
		  typename T> // The search type
struct Erase<TypeList<U, Ts...>, T> {
	using Type = typename Append<TypeList<U>, typename Erase<TypeList<Ts...>, T>::Type>::Type;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the Erase class template.
// \ingroup type_traits
//
// The Erase_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the Erase class template. For instance, given the type list \a TL and the type \a T the
// following two type definitions are identical:

   \code
   using Type1 = typename Erase<TL,T>::Type;
   using Type2 = Erase_t<TL,T>;
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The type to be appended to the type list
using Erase_t = typename Erase<TL, T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
