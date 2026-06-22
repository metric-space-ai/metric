// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVELVALUEREFERENCE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_REMOVELVALUEREFERENCE_H
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
// The RemoveLValueReference type trait removes an lvalue reference modifiers from the given
// type \a T.

   \code
   mtrc::numeric::RemoveLValueReference<int>::Type             // Results in 'int'
   mtrc::numeric::RemoveLValueReference<const int&>::Type      // Results in 'const int'
   mtrc::numeric::RemoveLValueReference<volatile int&&>::Type  // Results in 'volatile int&&'
   mtrc::numeric::RemoveLValueReference<int*>::Type            // Results in 'int*'
   mtrc::numeric::RemoveLValueReference<int*&>::Type           // Results in 'int*'
   mtrc::numeric::RemoveLValueReference<int*&&>::Type          // Results in 'int*&&'
   \endcode
*/
template <typename T> struct RemoveLValueReference {
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
//! Specialization of the RemoveLValueReference type trait for lvalue references.
template <typename T> struct RemoveLValueReference<T &> {
  public:
	//**********************************************************************************************
	using Type = T;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemoveLValueReference type trait.
// \ingroup type_traits
//
// The RemoveLValueReference_t alias declaration provides a convenient shortcut to access the
// nested \a Type of the RemoveLValueReference class template. For instance, given the type \a T
// the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemoveLValueReference<T>::Type;
   using Type2 = mtrc::numeric::RemoveLValueReference_t<T>;
   \endcode
*/
template <typename T> using RemoveLValueReference_t = typename RemoveLValueReference<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
