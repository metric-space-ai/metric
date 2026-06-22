// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVEEXTENT_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVEEXTENT_H
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
/*!\brief Removal of the top level array extent.
// \ingroup type_traits
//
// The RemoveExtent type trait removes the top level array extent from the given type \a T.

   \code
   mtrc::numeric::RemoveExtent<int>::Type           // Results in 'int'
   mtrc::numeric::RemoveExtent<int const[2]>::Type  // Results in 'int const'
   mtrc::numeric::RemoveExtent<int[2][4]>::Type     // Results in 'int[4]'
   mtrc::numeric::RemoveExtent<int[][2]>::Type      // Results in 'int[2]'
   mtrc::numeric::RemoveExtent<int const*>::Type    // Results in 'int const*'
   \endcode
*/
template <typename T> struct RemoveExtent {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::remove_extent<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemoveExtent type trait.
// \ingroup type_traits
//
// The RemoveExtent_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the RemoveExtent class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemoveExtent<T>::Type;
   using Type2 = mtrc::numeric::RemoveExtent_t<T>;
   \endcode
*/
template <typename T> using RemoveExtent_t = typename RemoveExtent<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
