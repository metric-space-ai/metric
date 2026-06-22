// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVECV_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVECV_H
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
// The RemoveCV type trait removes all top level cv-qualifiers from the given type \a T.

   \code
   mtrc::numeric::RemoveCV<short>::Type               // Results in 'short'
   mtrc::numeric::RemoveCV<const double>::Type        // Results in 'double'
   mtrc::numeric::RemoveCV<volatile float>::Type      // Results in 'float'
   mtrc::numeric::RemoveCV<const volatile int>::Type  // Results in 'int'
   mtrc::numeric::RemoveCV<int const*>::Type          // Results in 'int const*'
   mtrc::numeric::RemoveCV<int const* const>::Type    // Results in 'int const*'
   mtrc::numeric::RemoveCV<int const&>::Type          // Results in 'int const&'
   \endcode
*/
template <typename T> struct RemoveCV {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::remove_cv<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemoveCV type trait.
// \ingroup type_traits
//
// The RemoveCV_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the RemoveCV class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemoveCV<T>::Type;
   using Type2 = mtrc::numeric::RemoveCV_t<T>;
   \endcode
*/
template <typename T> using RemoveCV_t = typename RemoveCV<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
