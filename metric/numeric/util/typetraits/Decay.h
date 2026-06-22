// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_DECAY_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_DECAY_H
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
/*!\brief Applies the type conversions for by-value function arguments.
// \ingroup type_traits
//
// This type trait applies the type conversions that are used for by-value function arguments.
// This conversions include lvalue-to-rvalue, array-to-pointer, and function-to-pointer implicit
// conversions to the type \c T, and the removal of top level cv-qualifiers.

   \code
   mtrc::numeric::Decay<int>::Type         // Results in 'int'
   mtrc::numeric::Decay<int&>::Type        // Results in 'int'
   mtrc::numeric::Decay<int&&>::Type       // Results in 'int'
   mtrc::numeric::Decay<const int&>::Type  // Results in 'int'
   mtrc::numeric::Decay<int[2]>::Type      // Results in 'int*'
   mtrc::numeric::Decay<int(int)>::Type    // Results in 'int(*)(int)'
   \endcode
*/
template <typename T> struct Decay {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::decay<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the Decay type trait.
// \ingroup type_traits
//
// The Decay_t alias declaration provides a convenient shortcut to access the nested \a Type of
// the Decay class template. For instance, given the type \a T the following two type definitions
// are identical:

   \code
   using Type1 = typename mtrc::numeric::Decay<T>::Type;
   using Type2 = mtrc::numeric::Decay_t<T>;
   \endcode
*/
template <typename T> using Decay_t = typename Decay<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
