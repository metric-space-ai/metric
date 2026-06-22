// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_POLICIES_DEFAULTDELETE_H
#define METRIC_NUMERIC_UTIL_POLICIES_DEFAULTDELETE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/CheckedDelete.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Default C++ deletion policy class.
// \ingroup util
//
// The DefaultDelete deletion policy is the standard delete for resources allocated via the new
// operator. It uses delete or array delete (depending on the template argument) to free the
// resource:

   \code
   class Resource { ... };

   DefaultDelete<Resource> ptrDelete       // Uses delete to free resources
   DefaultDelete<Resource[]> arrayDelete;  // Uses array delete to free resources
   \endcode

// Note the explicit use of empty array bounds to configure DefaultDelete to use array delete
// instead of delete. Also note that the delete operation is NOT permitted for incomplete types
// (i.e. declared but undefined data types). The attempt to apply a DefaultDelete functor to a
// pointer or array to an object of incomplete type results in a compile time error!
*/
template <typename Type> struct DefaultDelete {
	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline void operator()(Type *ptr) const;
	//@}
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Implementation of the default delete policy.
//
// \param ptr The pointer to delete.
// \return void
//
// This function frees the given pointer resource via delete. Note that the delete operation
// is NOT permitted for incomplete types (i.e. declared but undefined data types). The attempt
// to use this function for a pointer to an object of incomplete type results in a compile time
// error!
*/
template <typename Type> inline void DefaultDelete<Type>::operator()(Type *ptr) const { checkedDelete(ptr); }
//*************************************************************************************************

//=================================================================================================
//
//  SPECIALIZATION FOR ARRAYS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DefaultDelete class template for arrays.
// \ingroup util
//
// This specialization of the DefaultDelete class template uses array delete to free the
// allocated resource.
*/
template <typename Type> struct DefaultDelete<Type[]> {
	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline void operator()(Type *ptr) const;
	//@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Implementation of the default array delete policy.
//
// \param ptr The pointer to delete.
// \return void
//
// This function frees the given array resource via array delete. Note that the delete operation
// is NOT permitted for incomplete types (i.e. declared but undefined data types). The attempt
// to use this function for a pointer to an object of incomplete type results in a compile time
// error!
*/
template <typename Type> inline void DefaultDelete<Type[]>::operator()(Type *ptr) const { checkedArrayDelete(ptr); }
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
