// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVEREFERENCE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVEREFERENCE_H
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
/*!\brief Removal of reference modifiers.
// \ingroup type_traits
//
// The RemoveReference type trait removes any reference modifiers from the given type \a T.

   \code
   mtrc::numeric::RemoveReference<int>::Type             // Results in 'int'
   mtrc::numeric::RemoveReference<const int&>::Type      // Results in 'const int'
   mtrc::numeric::RemoveReference<volatile int&&>::Type  // Results in 'volatile int'
   mtrc::numeric::RemoveReference<int*>::Type            // Results in 'int*'
   mtrc::numeric::RemoveReference<int*&>::Type           // Results in 'int*'
   \endcode
*/
template <typename T> struct RemoveReference {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::remove_reference<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemoveReference type trait.
// \ingroup type_traits
//
// The RemoveReference_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the RemoveReference class template. For instance, given the type \a T the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemoveReference<T>::Type;
   using Type2 = mtrc::numeric::RemoveReference_t<T>;
   \endcode
*/
template <typename T> using RemoveReference_t = typename RemoveReference<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
