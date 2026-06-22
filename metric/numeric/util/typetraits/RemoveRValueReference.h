// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVERVALUEREFERENCE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVERVALUEREFERENCE_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Removal of reference modifiers.
// \ingroup type_traits
//
// The RemoveRValueReference type trait removes an rvalue reference modifiers from the given
// type \a T.

   \code
   mtrc::numeric::RemoveRValueReference<int>::Type             // Results in 'int'
   mtrc::numeric::RemoveRValueReference<const int&>::Type      // Results in 'const int&'
   mtrc::numeric::RemoveRValueReference<volatile int&&>::Type  // Results in 'volatile int'
   mtrc::numeric::RemoveRValueReference<int*>::Type            // Results in 'int*'
   mtrc::numeric::RemoveRValueReference<int*&>::Type           // Results in 'int*&'
   mtrc::numeric::RemoveRValueReference<int*&&>::Type          // Results in 'int*'
   \endcode
*/
template <typename T> struct RemoveRValueReference {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = T;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
//! Specialization of the RemoveRValueReference type trait for rvalue references.
template <typename T> struct RemoveRValueReference<T &&> {
  public:
	//**********************************************************************************************
	using Type = T;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemoveRValueReference type trait.
// \ingroup type_traits
//
// The RemoveRValueReference_t alias declaration provides a convenient shortcut to access the
// nested \a Type of the RemoveRValueReference class template. For instance, given the type \a T
// the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemoveRValueReference<T>::Type;
   using Type2 = mtrc::numeric::RemoveRValueReference_t<T>;
   \endcode
*/
template <typename T> using RemoveRValueReference_t = typename RemoveRValueReference<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
