// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_POW2_H
#define METRIC_NUMERIC_MATH_SHIMS_POW2_H
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
//  POW2 SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Squaring the given value/object.
// \ingroup math_shims
//
// \param a The value/object to be squared.
// \return The result of the square operation.
//
// The \a pow2 shim represents an abstract interface for squaring a value/object of any given
// data type. For values of built-in data type this results in a plain multiplication.
*/
template <typename T, EnableIf_t<IsScalar_v<T> || IsSIMDPack_v<T>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) pow2(const T &a) noexcept(noexcept(a * a))
{
	return (a * a);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
