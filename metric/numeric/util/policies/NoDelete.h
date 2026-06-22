// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_POLICIES_NODELETE_H
#define METRIC_NUMERIC_UTIL_POLICIES_NODELETE_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief No-delete policy class.
// \ingroup util
*/
struct NoDelete {
	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	template <typename Type> inline void operator()(const Type &ptr) const;
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
/*!\brief Implementation of the no-delete policy.
//
// \param ptr The pointer to delete.
// \return void
*/
template <typename Type> inline void NoDelete::operator()(const Type & /*ptr*/) const {}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
