// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MISALIGNMENT_H
#define METRIC_NUMERIC_UTIL_MISALIGNMENT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/AlignmentOf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  SIZETRAIT CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computes the misalignment of the given address.
// \ingroup util
//
// \param address The address to be checked.
// \return The number of bytes the given address is misaligned.
//
// This function computes the misalignment of the given address with respect to the given data
// type \a Type and the available instruction set (SSE, AVX, ...). It returns the number of bytes
// the address is larger than the next smaller properly aligned address.
*/
template <typename T> METRIC_NUMERIC_ALWAYS_INLINE size_t misalignment(const T *address)
{
	return (reinterpret_cast<size_t>(address) % AlignmentOf_v<T>);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
