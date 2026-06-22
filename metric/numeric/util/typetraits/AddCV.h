// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ADDCV_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ADDCV_H
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
/*!\brief Addition of a top level 'const' and 'volatile' qualifier.
// \ingroup type_traits
//
// The AddCV type trait adds both a top level 'const' and 'volatile' qualifier to the given
// type \a T.

   \code
   mtrc::numeric::AddCV<int>::Type           // Results in 'int const volatile'
   mtrc::numeric::AddCV<int*>::Type          // Results in 'int* const volatile'
   mtrc::numeric::AddCV<int&>::Type          // Results in 'int&'
   mtrc::numeric::AddCV<int const>::Type     // Results in 'int const volatile'
   mtrc::numeric::AddCV<int volatile>::Type  // Results in 'int const volatile'
   \endcode
*/
template <typename T> struct AddCV {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::add_cv<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the AddCV type trait.
// \ingroup type_traits
//
// The AddCV_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the AddCV class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::AddCV<T>::Type;
   using Type2 = mtrc::numeric::AddCV_t<T>;
   \endcode
*/
template <typename T> using AddCV_t = typename AddCV<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
