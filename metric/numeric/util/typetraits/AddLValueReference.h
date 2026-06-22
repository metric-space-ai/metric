// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ADDLVALUEREFERENCE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ADDLVALUEREFERENCE_H
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
/*!\brief Addition of a top level reference.
// \ingroup type_traits
//
// In case the given type \a T is not a reference type, the AddLValueReference type trait adds
// a top level lvalue reference to the given type \a T. Else the resulting type \a Type is \a T.

   \code
   mtrc::numeric::AddLValueReference<int>::Type             // Results in 'int&'
   mtrc::numeric::AddLValueReference<int const&>::Type      // Results in 'int const&'
   mtrc::numeric::AddLValueReference<int volatile&&>::Type  // Results in 'int volatile&'
   mtrc::numeric::AddLValueReference<int*>::Type            // Results in 'int*&'
   mtrc::numeric::AddLValueReference<int*&>::Type           // Results in 'int*&'
   mtrc::numeric::AddLValueReference<int*&&>::Type          // Results in 'int*&'
   \endcode
*/
template <typename T> struct AddLValueReference {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::add_lvalue_reference<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the AddLValueReference type trait.
// \ingroup type_traits
//
// The AddLValueReference_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the AddLValueReference class template. For instance, given the type \a T the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::AddLValueReference<T>::Type;
   using Type2 = mtrc::numeric::AddLValueReference_t<T>;
   \endcode
*/
template <typename T> using AddLValueReference_t = typename AddLValueReference<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
