// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ADDCONST_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ADDCONST_H
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
/*!\brief Addition of a top level 'const' qualifier.
// \ingroup type_traits
//
// The AddConst type trait adds a top level 'const' qualifier to the given type \a T.

   \code
   mtrc::numeric::AddConst<int>::Type           // Results in 'int const'
   mtrc::numeric::AddConst<int*>::Type          // Results in 'int* const'
   mtrc::numeric::AddConst<int&>::Type          // Results in 'int&'
   mtrc::numeric::AddConst<int const>::Type     // Results in 'int const'
   mtrc::numeric::AddConst<int volatile>::Type  // Results in 'int const volatile'
   \endcode
*/
template <typename T> struct AddConst {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::add_const<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the AddConst type trait.
// \ingroup type_traits
//
// The AddConst_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the AddConst class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::AddConst<T>::Type;
   using Type2 = mtrc::numeric::AddConst_t<T>;
   \endcode
*/
template <typename T> using AddConst_t = typename AddConst<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
