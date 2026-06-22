// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ADDPOINTER_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ADDPOINTER_H
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
/*!\brief Addition of a top level pointer.
// \ingroup type_traits
//
// The AddPointer type trait adds a top level pointer to the given type \a T. It has the same
// effect as \c mtrc::numeric::RemovePointer<T>::Type*.

   \code
   mtrc::numeric::AddPointer<int>::Type        // Results in 'int*'
   mtrc::numeric::AddPointer<int const>::Type  // Results in 'int const*'
   mtrc::numeric::AddPointer<int*>::Type       // Results in 'int**'
   mtrc::numeric::AddPointer<int*&>::Type      // Results in 'int**'
   \endcode
*/
template <typename T> struct AddPointer {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::add_pointer<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the AddPointer type trait.
// \ingroup type_traits
//
// The AddPointer_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the AddPointer class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::AddPointer<T>::Type;
   using Type2 = mtrc::numeric::AddPointer_t<T>;
   \endcode
*/
template <typename T> using AddPointer_t = typename AddPointer<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
