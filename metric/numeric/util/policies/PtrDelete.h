// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_POLICIES_PTRDELETE_H
#define METRIC_NUMERIC_UTIL_POLICIES_PTRDELETE_H
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
/*!\brief Pointer-delete policy class.
// \ingroup util
//
// The PtrDelete policy functor class applies a delete operation to the given argument. Note that
// the delete operation is NOT permitted for inclomplete types (i.e. declared but undefined data
// types). The attempt to apply a PtrDelete functor to a pointer to an object of incomplete type
// results in a compile time error!
*/
struct PtrDelete {
	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	template <typename Type> inline void operator()(Type ptr) const;
	//@}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Implementation of the pointer-delete policy.
//
// \param ptr The pointer to delete.
// \return void
//
// This function applies a standard delete operation to the given argument. Note that the delete
// operation is NOT permitted for inclomplete types (i.e. declared but undefined data types). The
// attempt to use this function for a pointer to an object of incomplete type results in a compile
// time error!
*/
template <typename Type> inline void PtrDelete::operator()(Type ptr) const { checkedDelete(ptr); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
