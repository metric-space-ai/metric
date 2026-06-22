// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_LOG2_H
#define METRIC_NUMERIC_MATH_FUNCTORS_LOG2_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/constraints/SIMDPack.h>
#include <metric/numeric/math/shims/Log2.h>
#include <metric/numeric/math/simd/Log2.h>
#include <metric/numeric/math/typetraits/HasSIMDLog2.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
#include <metric/numeric/math/typetraits/YieldsSymmetric.h>
#include <metric/numeric/math/typetraits/YieldsUniform.h>
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
/*!\brief Generic wrapper for the log2() function.
// \ingroup functors
*/
struct Log2 {
	//**********************************************************************************************
	/*!\brief Returns the result of the log2() function for the given object/value.
	//
	// \param a The given object/value.
	// \return The result of the log2() function for the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T &&a) const
	{
		return log2(std::forward<T>(a));
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data type \a T.
	//
	// \return \a true in case SIMD is enabled for the data type \a T, \a false if not.
	*/
	template <typename T> static constexpr bool simdEnabled() { return HasSIMDLog2_v<T>; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the operation supports padding, i.e. whether it can deal with zeros.
	//
	// \return \a true in case padding is supported, \a false if not.
	*/
	static constexpr bool paddingEnabled() { return false; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the log2() function for the given SIMD vector.
	//
	// \param a The given SIMD vector.
	// \return The result of the log2() function for the given SIMD vector.
	*/
	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(const T &a) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T);
		return log2(a);
	}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUNIFORM SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> struct YieldsUniform<Log2, T> : public IsUniform<T> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsSymmetric<Log2, MT> : public IsSymmetric<MT> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
