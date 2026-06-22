// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVEALLEXTENTS_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVEALLEXTENTS_H
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
/*!\brief Removal of all array extents.
// \ingroup type_traits
//
// The RemoveAllExtents type trait removes all array extents from the given type \a T.

   \code
   mtrc::numeric::RemoveAllExtents<int>::Type           // Results in 'int'
   mtrc::numeric::RemoveAllExtents<int const[2]>::Type  // Results in 'int const'
   mtrc::numeric::RemoveAllExtents<int[2][4]>::Type     // Results in 'int'
   mtrc::numeric::RemoveAllExtents<int[][2]>::Type      // Results in 'int'
   mtrc::numeric::RemoveAllExtents<int[2][3][4]>::Type  // Results in 'int'
   mtrc::numeric::RemoveAllExtents<int const*>::Type    // Results in 'int const*'
   \endcode
*/
template <typename T> struct RemoveAllExtents {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::remove_all_extents<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemoveAllExtents type trait.
// \ingroup type_traits
//
// The RemoveAllExtents_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the RemoveAllExtents class template. For instance, given the type \a T the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemoveAllExtents<T>::Type;
   using Type2 = mtrc::numeric::RemoveAllExtents_t<T>;
   \endcode
*/
template <typename T> using RemoveAllExtents_t = typename RemoveAllExtents<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
