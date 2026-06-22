// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPELIST_ERASEALL_H
#define METRIC_NUMERIC_UTIL_TYPELIST_ERASEALL_H
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
/*!\brief Erasing all occurrences of a type from a type list.
// \ingroup typelist
//
// The EraseAll class can be used to erase all occurrences of data type \a Type from a type list
// \a TList. In order to erase all occurrences of a data type, the EraseAll class has to be
// instantiated for a particular type list and another type. The following example gives an
// impression of the use of the EraseAll class:

   \code
   // Defining a temporary type list containing the type int twice
   using Tmp = mtrc::numeric::TypeList< float, int, double, int >;

   // Erasing the all occurrences of int from the type list
   using NoInt = mtrc::numeric::EraseAll<Tmp,int>::Type;
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The type to be erased from the type list
struct EraseAll;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the EraseAll class for empty type lists.
// \ingroup typelist
*/
template <typename T> // The type to be erased from the type list
struct EraseAll<TypeList<>, T> {
	using Type = TypeList<>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the EraseAll class for erasing an occurrence of T.
// \ingroup typelist
*/
template <typename T // The type to be erased from the type list
		  ,
		  typename... Ts> // Type of the tail of the type list
struct EraseAll<TypeList<T, Ts...>, T> {
	using Type = typename EraseAll<TypeList<Ts...>, T>::Type;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the EraseAll class for a general type list.
// \ingroup typelist
*/
template <typename U // Type of the head of the type list
		  ,
		  typename... Ts // Types of the tail of the type list
		  ,
		  typename T> // The search type
struct EraseAll<TypeList<U, Ts...>, T> {
	using Type = typename Append<TypeList<U>, typename EraseAll<TypeList<Ts...>, T>::Type>::Type;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the EraseAll class template.
// \ingroup type_traits
//
// The EraseAll_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the EraseAll class template. For instance, given the type list \a TL and the type \a T the
// following two type definitions are identical:

   \code
   using Type1 = typename EraseAll<TL,T>::Type;
   using Type2 = EraseAll_t<TL,T>;
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The type to be erased from the type list
using EraseAll_t = typename EraseAll<TL, T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
