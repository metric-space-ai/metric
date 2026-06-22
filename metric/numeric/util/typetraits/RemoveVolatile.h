// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVEVOLATILE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVEVOLATILE_H
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
/*!\brief Removal of volatile-qualifiers.
// \ingroup type_traits
//
// The RemoveVolatile type trait removes all top level 'volatile' qualifiers from the given
// type \a T.

   \code
   mtrc::numeric::RemoveVolatile<short>::Type                   // Results in 'short'
   mtrc::numeric::RemoveVolatile<volatile double>::Type         // Results in 'double'
   mtrc::numeric::RemoveVolatile<const volatile int>::Type      // Results in 'const int'
   mtrc::numeric::RemoveVolatile<int volatile*>::Type           // Results in 'int volatile*'
   mtrc::numeric::RemoveVolatile<int volatile* volatile>::Type  // Results in 'int volatile*'
   mtrc::numeric::RemoveVolatile<int volatile&>::Type           // Results in 'int volatile&'
   \endcode
*/
template <typename T> struct RemoveVolatile {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::remove_volatile<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemoveVolatile type trait.
// \ingroup type_traits
//
// The RemoveVolatile_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the RemoveVolatile class template. For instance, given the type \a T the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemoveVolatile<T>::Type;
   using Type2 = mtrc::numeric::RemoveVolatile_t<T>;
   \endcode
*/
template <typename T> using RemoveVolatile_t = typename RemoveVolatile<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
