// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_POLICIES_ARRAYDELETE_H
#define METRIC_NUMERIC_UTIL_POLICIES_ARRAYDELETE_H
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
/*!\brief Array-delete policy class.
// \ingroup util
//
// The ArrayDelete policy functor class applies an array delete operation to the given argument.
// Note that the array delete operation is NOT permitted for inclomplete types (i.e. declared
// but undefined data types). The attempt to apply an ArrayDelete functor to a pointer to an
// array of objects of incomplete type results in a compile time error!
*/
struct ArrayDelete {
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
/*!\brief Implementation of the array-delete policy.
//
// \param ptr The pointer to the array to be deleted.
// \return void
//
// This function applies an array delete operation to the given argument. Note that the array
// delete operation is NOT permitted for inclomplete types (i.e. declared but undefined data
// types). The attempt to use this function for a pointer to an array of objects of incomplete
// type results in a compile time error!
*/
template <typename Type> inline void ArrayDelete::operator()(Type ptr) const { checkedArrayDelete(ptr); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
