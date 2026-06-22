// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPELIST_CONTAINSRELATED_H
#define METRIC_NUMERIC_UTIL_TYPELIST_CONTAINSRELATED_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typelist/TypeList.h>
#include <metric/numeric/util/typetraits/IsConvertible.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS TEMPLATE
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Searching a type list.
// \ingroup typelist
//
// The ContainsRelated class can be used to search the type list for a type related to \a Type.
// In contrast to the Contains class, the ContainsRelated class only searches for a type the
// given data type \a Type can be converted to. In case a related type is found in the type list,
// the \a value member enumeration is set to \a true, else it is set to \a false. In order to
// check whether a related type is contained in the type list, the ContainsRelated class has to
// be instantiated for a particular type list and another type. The following example gives an
// impression of the use of the ContainsRelated class:

   \code
   class A {};
   class B : public A {};
   class C {};
   class D {};

   // Defining a new type list
   using Types = mtrc::numeric::TypeList< A, C >;

   // Searching for the type A in the type list
   constexpr bool a = mtrc::numeric::ContainsRelated< Types, A >::value;  // Evaluates to 1, type A is found

   // Searching for the derived type B in the type list
   constexpr bool b = mtrc::numeric::ContainsRelated< Types, B >::value;  // Evaluates to 1, base type A is found

   // Searching for the type C in the type list
   constexpr bool c = mtrc::numeric::ContainsRelated< Types, D >::value;  // Evaluates to 0, no related type found
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The search type
struct ContainsRelated;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the ContainsRelated class for empty type lists.
// \ingroup typelist
*/
template <typename T> // The search type
struct ContainsRelated<TypeList<>, T> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Spezialization of the ContainsRelated class for a general type list.
// \ingroup typelist
*/
template <typename U // Type of the head of the type list
		  ,
		  typename... Ts // Types of the tail of the type list
		  ,
		  typename T> // The search type
struct ContainsRelated<TypeList<U, Ts...>, T>
	: public If_t<IsConvertible_v<T, U>, TrueType, ContainsRelated<TypeList<Ts...>, T>> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the ContainsRelated type trait.
// \ingroup type_traits
//
// The ContainsRelated_v variable template provides a convenient shortcut to access the nested
// \a value of the ContainsRelated class template. For instance, given the type list \a TL and
// the type \a T the following two statements are identical:

   \code
   constexpr bool value1 = ContainsRelated<TL,T>::value;
   constexpr bool value2 = ContainsRelated_v<TL,T>;
   \endcode
*/
template <typename TL // Type of the type list
		  ,
		  typename T> // The search type
constexpr bool ContainsRelated_v = ContainsRelated<TL, T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
