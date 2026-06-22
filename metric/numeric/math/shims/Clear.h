// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_CLEAR_H
#define METRIC_NUMERIC_MATH_SHIMS_CLEAR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsSIMDPack.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/EnableIf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLEAR SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Clearing the given value/object to the default state.
// \ingroup math_shims
//
// \param clearable The value/object to be cleared.
// \return void
//
// The \a clear shim represents an abstract interface for clearing a value/object of any given
// data type to its default state. Values of built-in data type are reset to zero.
*/
template <typename T, EnableIf_t<IsScalar_v<T> || IsSIMDPack_v<T>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE constexpr void clear(T &clearable)
{
	clearable = T();
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
