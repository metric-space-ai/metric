// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ALIGNMENTCHECK_H
#define METRIC_NUMERIC_UTIL_ALIGNMENTCHECK_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/Misalignment.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  SIZETRAIT CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Checks the alignment of the given address.
// \ingroup util
//
// \param address The address to be checked.
// \return \a true in case the address is properly aligned, \a false if it is not.
//
// This function performs an alignment check on the given address. For instance, for fundamental
// data types that can be vectorized via SSE or AVX instructions, the proper alignment is 16 or
// 32 bytes, respectively. In case the given address is properly aligned, the function returns
// \a true, otherwise it returns \a false.
*/
template <typename T> METRIC_NUMERIC_ALWAYS_INLINE bool checkAlignment(const T *address)
{
	return (misalignment(address) == 0UL);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
