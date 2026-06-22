// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_POW_H
#define METRIC_NUMERIC_MATH_FUNCTORS_POW_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/constraints/SIMDPack.h>
#include <metric/numeric/math/shims/Pow.h>
#include <metric/numeric/math/simd/Pow.h>
#include <metric/numeric/math/typetraits/HasSIMDPow.h>
#include <metric/numeric/system/HostDevice.h>
#include <metric/numeric/system/Inline.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for the pow() function.
// \ingroup functors
*/
struct Pow {
	//**********************************************************************************************
	/*!\brief Returns the result of the pow() function for the given objects/values.
	//
	// \param a The left-hand side object/value.
	// \param b The right-hand side object/value.
	// \return The result of the pow() function for the given objects/values.
	*/
	template <typename T1, typename T2>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T1 &&a, T2 &&b) const
	{
		return pow(std::forward<T1>(a), std::forward<T2>(b));
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data types \a T1 and \a T2.
	//
	// \return \a true in case SIMD is enabled for the data types \a T1 and \a T2, \a false if not.
	*/
	template <typename T1, typename T2> static constexpr bool simdEnabled() { return HasSIMDPow_v<T1, T2>; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the operation supports padding, i.e. whether it can deal with zeros.
	//
	// \return \a true in case padding is supported, \a false if not.
	*/
	static constexpr bool paddingEnabled() { return false; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the pow() function for the given SIMD vector.
	//
	// \param a The left-hand side SIMD vector.
	// \param b The right-hand side SIMD vector.
	// \return The result of the pow() function for the given SIMD vector.
	*/
	template <typename T1, typename T2> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(const T1 &a, const T2 &b) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T1);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T2);
		return pow(a, b);
	}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
