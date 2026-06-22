// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_POW4_H
#define METRIC_NUMERIC_MATH_SHIMS_POW4_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/shims/Pow2.h>
#include <metric/numeric/math/typetraits/IsSIMDPack.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/EnableIf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  POW4 SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Quadruple the given value/object.
// \ingroup math_shims
//
// \param a The value/object to be quadrupled.
// \return The result of the quadruple operation.
//
// The \a pow4 shim represents an abstract interface for quadrupling a value/object of any
// given data type. For values of built-in data type this results in a plain multiplication.
*/
template <typename T, EnableIf_t<IsScalar_v<T> || IsSIMDPack_v<T>> * = nullptr>
METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) pow4(const T &a) noexcept(noexcept(pow2(pow2(a))))
{
	return pow2(pow2(a));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
