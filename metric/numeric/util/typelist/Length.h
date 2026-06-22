// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPELIST_LENGTH_H
#define METRIC_NUMERIC_UTIL_TYPELIST_LENGTH_H
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
/*!\brief Calculating the length of a type list.
// \ingroup typelist
//
// The Length class can be used to obtain the length of a type list (i.e. the number
// of contained types). In order to obtain the length of a type list, the Length class
// has to be instantiated for a particular type list. The length of the type list can
// be obtained using the member enumeration \a value. The following example gives an
// impression of the use of the Length class:

   \code
   using Floats = mtrc::numeric::TypeList< float, double, long double >;  // Defining a new type list
   const int length = mtrc::numeric::Length< Floats >::value;             // The length of the type list
   \endcode
*/
template <typename TL> // Type of the type list
struct Length;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the Length class for type lists.
// \ingroup typelist
*/
template <typename... Ts> // Type list elements
struct Length<TypeList<Ts...>> : public Size_t<sizeof...(Ts)> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Length type trait.
// \ingroup type_traits
//
// The Length_v variable template provides a convenient shortcut to access the nested \a value of
// the Length class template. For instance, given the type list \a TL the following two statements
// are identical:

   \code
   constexpr size_t value1 = Length<TL>::value;
   constexpr size_t value2 = Length_v<TL>;
   \endcode
*/
template <typename TL> // Type of the type list
constexpr size_t Length_v = Length<TL>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
