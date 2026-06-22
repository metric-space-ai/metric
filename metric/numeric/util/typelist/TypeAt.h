// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPELIST_TYPEAT_H
#define METRIC_NUMERIC_UTIL_TYPELIST_TYPEAT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/InvalidType.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typelist/TypeList.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Indexing a type list.
// \ingroup typelist
//
// The TypeAt class can be used to access a type list at a specified position to query the
// according type. In order to index a type list, the TypeAt class has to be instantiated
// for a particular type list and an index value. The indexed type is available via the
// member type definition \a Type. The following example gives an impression of the use
// of the TypeAt class:

   \code
   using Floats = mtrc::numeric::TypeList< float, double, long double >;  // Defining a new type list
   using Index0 = mtrc::numeric::TypeAt< Floats, 0 >::Type;               // Indexing of the type list at index 0
   \endcode

// \note The access index is zero based!
*/
template <typename TL // Type of the type list
		  ,
		  size_t Index> // Type list access index
struct TypeAt;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the TypeAt class for an index of 0.
// \ingroup typelist
*/
template <typename T // Type at the head of the type list
		  ,
		  typename... Ts> // Types of the tail of the type list
struct TypeAt<TypeList<T, Ts...>, 0UL> {
	using Type = T;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the TypeAt class for empty type lists.
// \ingroup typelist
*/
template <size_t Index> // Type list access index
struct TypeAt<TypeList<>, Index> {
	using Type = INVALID_TYPE;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the TypeAt class for a general index.
// \ingroup typelist
*/
template <typename T // Type of the head of the type list
		  ,
		  typename... Ts // Types of the tail of the type list
		  ,
		  size_t Index> // Type list access index
struct TypeAt<TypeList<T, Ts...>, Index> {
	using Type = typename TypeAt<TypeList<Ts...>, Index - 1UL>::Type;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the TypeAt class template.
// \ingroup type_traits
//
// The TypeAt_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the TypeAt class template. For instance, given the type list \a TL and the index \a Index
// the following two type definitions are identical:

   \code
   using Type1 = typename TypeAt<TL,Index>::Type;
   using Type2 = TypeAt_t<TL,Index>;
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  size_t Index> // Type list access index
using TypeAt_t = typename TypeAt<TL, Index>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
