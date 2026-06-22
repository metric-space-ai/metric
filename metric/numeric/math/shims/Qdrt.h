// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_QDRT_H
#define METRIC_NUMERIC_MATH_SHIMS_QDRT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>

namespace mtrc::numeric {

//=================================================================================================
//
//  QDRT SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computing the quad root (4th root) of the given value/object.
// \ingroup math_shims
//
// \param a The value/object for the computation.
// \return The quad root (4th root) of the given value/object.
//
// The \a qdrt shim represents an abstract interface for computing the quad root (i.e. 4th root)
// of a value/object of any given data type.
*/
template <typename T> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) qdrt(const T &a) { return sqrt(sqrt(a)); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
