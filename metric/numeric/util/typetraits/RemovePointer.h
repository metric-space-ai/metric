// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVEPOINTER_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVEPOINTER_H
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
/*!\brief Removal of pointer modifiers.
// \ingroup type_traits
//
// The RemovePointer type trait removes any pointer modifiers from the given type \a T.

   \code
   mtrc::numeric::RemovePointer<int>::Type             // Results in 'int'
   mtrc::numeric::RemovePointer<const int*>::Type      // Results in 'const int'
   mtrc::numeric::RemovePointer<volatile int**>::Type  // Results in 'volatile int*'
   mtrc::numeric::RemovePointer<int&>::Type            // Results in 'int&'
   mtrc::numeric::RemovePointer<int*&>::Type           // Results in 'int*&'
   \endcode
*/
template <typename T> struct RemovePointer {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::remove_pointer<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemovePointer type trait.
// \ingroup type_traits
//
// The RemovePointer_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the RemovePointer class template. For instance, given the type \a T the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemovePointer<T>::Type;
   using Type2 = mtrc::numeric::RemovePointer_t<T>;
   \endcode
*/
template <typename T> using RemovePointer_t = typename RemovePointer<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
