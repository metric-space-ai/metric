// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVECONST_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVECONST_H
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
/*!\brief Removal of const-qualifiers.
// \ingroup type_traits
//
// The RemoveConst type trait removes all top level 'const' qualifiers from the given type \a T.

   \code
   mtrc::numeric::RemoveConst<short>::Type               // Results in 'short'
   mtrc::numeric::RemoveConst<const double>::Type        // Results in 'double'
   mtrc::numeric::RemoveConst<const volatile int>::Type  // Results in 'volatile int'
   mtrc::numeric::RemoveConst<int const*>::Type          // Results in 'const int*'
   mtrc::numeric::RemoveConst<int const* const>::Type    // Results in 'const int*'
   mtrc::numeric::RemoveConst<int const&>::Type          // Results in 'const int&'
   \endcode
*/
template <typename T> struct RemoveConst {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::remove_const<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemoveConst type trait.
// \ingroup type_traits
//
// The RemoveConst_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the RemoveConst class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemoveConst<T>::Type;
   using Type2 = mtrc::numeric::RemoveConst_t<T>;
   \endcode
*/
template <typename T> using RemoveConst_t = typename RemoveConst<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
