// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVECVREF_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVECVREF_H
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
/*!\brief Removal of top level cv-qualifiers.
// \ingroup type_traits
//
// The RemoveCVRef type trait removes any reference modifiers and all top level cv-qualifiers
// from the given type \a T.

   \code
   mtrc::numeric::RemoveCVRef<short>::Type               // Results in 'short'
   mtrc::numeric::RemoveCVRef<int&>::Type                // Results in 'int'
   mtrc::numeric::RemoveCVRef<const double>::Type        // Results in 'double'
   mtrc::numeric::RemoveCVRef<volatile float>::Type      // Results in 'float'
   mtrc::numeric::RemoveCVRef<const volatile int>::Type  // Results in 'int'
   mtrc::numeric::RemoveCVRef<const int&>::Type          // Results in 'int'
   mtrc::numeric::RemoveCVRef<const int*>::Type          // Results in 'int const*'
   mtrc::numeric::RemoveCVRef<const int* const>::Type    // Results in 'int const*'
   \endcode
*/
template <typename T> struct RemoveCVRef {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemoveCVRef type trait.
// \ingroup type_traits
//
// The RemoveCVRef_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the RemoveCVRef class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemoveCVRef<T>::Type;
   using Type2 = mtrc::numeric::RemoveCVRef_t<T>;
   \endcode
*/
template <typename T> using RemoveCVRef_t = typename RemoveCVRef<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
