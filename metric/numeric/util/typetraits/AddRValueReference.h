// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ADDRVALUEREFERENCE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ADDRVALUEREFERENCE_H
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
// In case the given type \a T is not a reference type, the AddRValueReference type trait adds
// a top level rvalue reference to the given type \a T. Else the resulting type \a Type is \a T.

   \code
   mtrc::numeric::AddRValueReference<int>::Type         // Results in 'int&'
   mtrc::numeric::AddRValueReference<int const&>::Type  // Results in 'int const&'
   mtrc::numeric::AddRValueReference<int*>::Type        // Results in 'int*&'
   mtrc::numeric::AddRValueReference<int*&>::Type       // Results in 'int*&'
   \endcode
*/
template <typename T> struct AddRValueReference {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::add_rvalue_reference<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the AddRValueReference type trait.
// \ingroup type_traits
//
// The AddRValueReference_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the AddRValueReference class template. For instance, given the type \a T the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::AddRValueReference<T>::Type;
   using Type2 = mtrc::numeric::AddRValueReference_t<T>;
   \endcode
*/
template <typename T> using AddRValueReference_t = typename AddRValueReference<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
