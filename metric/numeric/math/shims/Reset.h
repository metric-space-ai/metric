// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_RESET_H
#define METRIC_NUMERIC_MATH_SHIMS_RESET_H
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
//  RESET SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Resetting the given value/object to the default value.
// \ingroup math_shims
//
// \param resettable The value/object to be resetted.
// \return void
//
// The \a reset shim represents an abstract interface for the resetting of a value/object of
// any given data type to its default value. Values of built-in data type are reset to zero.
*/
template <typename T, EnableIf_t<IsScalar_v<T> || IsSIMDPack_v<T>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE constexpr void reset(T &resettable)
{
	resettable = T();
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
