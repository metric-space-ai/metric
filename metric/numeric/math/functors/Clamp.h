// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_CLAMP_H
#define METRIC_NUMERIC_MATH_FUNCTORS_CLAMP_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/constraints/SIMDPack.h>
#include <metric/numeric/math/shims/Clamp.h>
#include <metric/numeric/math/simd/Max.h>
#include <metric/numeric/math/simd/Min.h>
#include <metric/numeric/math/simd/Set.h>
#include <metric/numeric/math/typetraits/HasSIMDMax.h>
#include <metric/numeric/math/typetraits/HasSIMDMin.h>
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
/*!\brief Generic wrapper for the clamp() function.
// \ingroup functors
*/
struct Clamp {
	//**********************************************************************************************
	/*!\brief Returns the result of the clamp() function for the given object/value.
	//
	// \param v The given object/value to clamp.
	// \param lo The minimum to clamp \a v.
	// \param hi The maximum to clamp \a v.
	// \return The result of the clamp() function for the given object/value.
	*/
	template <typename T1, typename T2, typename T3>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T1 &&v, T2 &&lo,
																						  T3 &&hi) const
	{
		return clamp(std::forward<T1>(v), std::forward<T2>(lo), std::forward<T3>(hi));
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data type \a T.
	//
	// \return \a true in case SIMD is enabled for the data type \a T, \a false if not.
	*/
	template <typename T1, typename T2, typename T3> static constexpr bool simdEnabled()
	{
		return HasSIMDMax_v<T1, T2> && HasSIMDMin_v<T1, T3>;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the operation supports padding, i.e. whether it can deal with zeros.
	//
	// \return \a true in case padding is supported, \a false if not.
	*/
	static constexpr bool paddingEnabled() { return true; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the clamp() function for the given SIMD vector.
	//
	// \param v The SIMD vector to clamp.
	// \param lo The minimum to clamp \a v.
	// \param hi The maximum to clamp \a v.
	// \return The result of the clamp() function for the given SIMD vector.
	*/
	template <typename T1, typename T2, typename T3>
	METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(const T1 &v, const T2 &lo, const T3 &hi) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T1);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T2);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T3);
		return min(max(v, lo), hi);
	}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
