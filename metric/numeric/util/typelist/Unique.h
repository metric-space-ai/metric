// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPELIST_UNIQUE_H
#define METRIC_NUMERIC_UTIL_TYPELIST_UNIQUE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typelist/Append.h>
#include <metric/numeric/util/typelist/Erase.h>
#include <metric/numeric/util/typelist/TypeList.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Erasing all duplicates from a type list.
// \ingroup typelist
//
// The Unique class can be used to erase all duplicates from a type list \a TList. In order to
// erase all duplicates, the Unique class has to be instantiated for a particular type list.
// The following example gives an impression of the use of the Unique class:

   \code
   // Defining a temporary type list containing the types int and float twice
   using Tmp = mtrc::numeric::TypeList< float, int, double, int, float >;

   // Removing all duplicates from the type list
   using NoDuplicates = mtrc::numeric::Unique<Tmp>::Type;
   \endcode
*/
template <typename TL> // Type of the type list
struct Unique;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Unique class for an empty type list.
// \ingroup typelist
*/
template <> struct Unique<TypeList<>> {
	using Type = TypeList<>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Unique class for a general type list.
// \ingroup typelist
*/
template <typename T // Type of the head of the type list
		  ,
		  typename... Ts> // Types of the tail of the type list
struct Unique<TypeList<T, Ts...>> {
  private:
	using TL1 = typename Unique<TypeList<Ts...>>::Type;
	using TL2 = typename Erase<TL1, T>::Type;

  public:
	using Type = typename Append<TypeList<T>, TL2>::Type;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the Unique class template.
// \ingroup type_traits
//
// The Unique_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the Unique class template. For instance, given the type list \a TL the following two type
// definitions are identical:

   \code
   using Type1 = typename Unique<TL>::Type;
   using Type2 = Unique_t<TL>;
   \endcode
*/
template <typename TL> // Type of the type list
using Unique_t = typename Unique<TL>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
