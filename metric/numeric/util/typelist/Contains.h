// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPELIST_CONTAINS_H
#define METRIC_NUMERIC_UTIL_TYPELIST_CONTAINS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typelist/TypeList.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Searching a type list.
// \ingroup typelist
//
// The Contains class can be used to search the type list for a particular type \a Type. In
// contrast to the IndexOf class, the Contains class does not evaluate the index of the type but
// only checks whether or not the type is contained in the type list. Additionally, in contrast
// to the ContainsRelated class, the Contains class strictly searches for the given type \a Type
// and not for a related data type. In case the type is contained in the type list, the \a value
// member enumeration is set to \a true, else it is set to \a false. In order to check whether a
// type is part of a type list, the Contains class has to be instantiated for a particular type
// list and another type. The following example gives an impression of the use of the Contains
// class:

   \code
   using Floats = mtrc::numeric::TypeList< float, double, long double >;      // Defining a new type list
   constexpr bool index1 = mtrc::numeric::Contains< Floats, double >::value;  // Value evaluates to true
   constexpr bool index2 = mtrc::numeric::Contains< Floats, int    >::value;  // Value evaluates to false
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The search type
struct Contains;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Contains class for empty type lists.
// \ingroup typelist
*/
template <typename T> // The search type
struct Contains<TypeList<>, T> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Contains class for a successful search.
// \ingroup typelist
*/
template <typename T // The search type
		  ,
		  typename... Ts> // Types of the tail of the type list
struct Contains<TypeList<T, Ts...>, T> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Contains class for a general type list.
// \ingroup typelist
*/
template <typename U // Type of the head of the type list
		  ,
		  typename... Ts // Types of the tail of the type list
		  ,
		  typename T> // The search type
struct Contains<TypeList<U, Ts...>, T> : public Contains<TypeList<Ts...>, T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Contains type trait.
// \ingroup type_traits
//
// The Contains_v variable template provides a convenient shortcut to access the nested \a value
// of the Contains class template. For instance, given the type list \a TL and the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = Contains<TL,T>::value;
   constexpr bool value2 = Contains_v<TL,T>;
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The search type
constexpr bool Contains_v = Contains<TL, T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
