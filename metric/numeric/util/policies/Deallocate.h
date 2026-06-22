// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_POLICIES_DEALLOCATE_H
#define METRIC_NUMERIC_UTIL_POLICIES_DEALLOCATE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Memory.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Deallocate policy class.
// \ingroup util
//
// The Deallocate deletion policy is the according deletion policy for arrays allocated via
// the mtrc::numeric::allocate function. It uses deallocate to free the resource. Note that the delete
// operation is NOT permitted for inclomplete types (i.e. declared but undefined data types).
// The attempt to apply a PtrDelete functor to a pointer to an object of incomplete type
// results in a compile time error!
*/
struct Deallocate {
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
/*!\brief Implementation of the deallocate deletion policy.
//
// \param ptr The pointer to delete.
// \return void
//
// This function frees the given pointer resource via the mtrc::numeric::deallocate function. Note that
// the delete operation is NOT permitted for incomplete types (i.e. declared but undefined data
// types). The attempt to use this function for a pointer to an object of incomplete type results
// in a compile time error!
*/
template <typename Type> inline void Deallocate::operator()(Type ptr) const { deallocate(ptr); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
