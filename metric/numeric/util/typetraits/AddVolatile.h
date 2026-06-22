// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ADDVOLATILE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ADDVOLATILE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <type_traits>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Addition of a top level 'volatile' qualifier.
// \ingroup type_traits
//
// The AddVolatile type trait adds a top level 'volatile' qualifier to the given type \a T.

   \code
   mtrc::numeric::AddVolatile<int>::Type           // Results in 'int volatile'
   mtrc::numeric::AddVolatile<int*>::Type          // Results in 'int* volatile'
   mtrc::numeric::AddVolatile<int&>::Type          // Results in 'int&'
   mtrc::numeric::AddVolatile<int volatile>::Type  // Results in 'int volatile'
   mtrc::numeric::AddVolatile<int const>::Type     // Results in 'int const volatile'
   \endcode
*/
template <typename T> struct AddVolatile {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::add_volatile<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the AddVolatile type trait.
// \ingroup type_traits
//
// The AddVolatile_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the AddVolatile class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::AddVolatile<T>::Type;
   using Type2 = mtrc::numeric::AddVolatile_t<T>;
   \endcode
*/
template <typename T> using AddVolatile_t = typename AddVolatile<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
