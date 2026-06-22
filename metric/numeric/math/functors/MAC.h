// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_MAC_H
#define METRIC_NUMERIC_MATH_FUNCTORS_MAC_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/constraints/SIMDPack.h>
#include <metric/numeric/math/typetraits/HasSIMDAdd.h>
#include <metric/numeric/math/typetraits/HasSIMDMult.h>
#include <metric/numeric/math/typetraits/IsLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
#include <metric/numeric/math/typetraits/IsUpper.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/math/typetraits/YieldsLower.h>
#include <metric/numeric/math/typetraits/YieldsStrictlyLower.h>
#include <metric/numeric/math/typetraits/YieldsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/YieldsUniform.h>
#include <metric/numeric/math/typetraits/YieldsUpper.h>
#include <metric/numeric/math/typetraits/YieldsZero.h>
#include <metric/numeric/system/HostDevice.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for the multiply-accumulate (\f$ (a*b)+c \f$; MAC) operation.
// \ingroup functors
*/
struct MAC {
	//**********************************************************************************************
	/*!\brief Returns the result of the MAC operation for the given objects/values.
	//
	// \param a The first MAC operand.
	// \param b The second MAC operand.
	// \param c The third MAC operand
	// \return The result of the MAC operation for the given objects/values.
	*/
	template <typename T1, typename T2, typename T3>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T1 &&a, T2 &&b, T3 &&c) const
	{
		return (std::forward<T1>(a) * std::forward<T2>(b)) + std::forward<T3>(c);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data types \a T1, \a T2, and \a T3.
	//
	// \return \a true in case SIMD is enabled for the data types \a T1, \a T2, and \a T3, \a false if not.
	*/
	template <typename T1, typename T2, typename T3> static constexpr bool simdEnabled()
	{
		return HasSIMDMult_v<T1, T2> && HasSIMDAdd_v<T1, T3>;
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
	/*!\brief Returns the result of the MAC operation for the given SIMD vectors.
	//
	// \param a The first SIMD vector.
	// \param b The second SIMD vector.
	// \param c The third SIMD vector.
	// \return The result of the MAC operation for the given SIMD vectors.
	*/
	template <typename T1, typename T2, typename T3>
	METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(const T1 &a, const T2 &b, const T3 &c) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T1);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T2);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T3);
		return (a * b) + c;
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
template <typename T1, typename T2, typename T3>
struct YieldsUniform<MAC, T1, T2, T3> : public BoolConstant<IsUniform_v<T1> && IsUniform_v<T2> && IsUniform_v<T3>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2, typename MT3>
struct YieldsLower<MAC, MT1, MT2, MT3> : public BoolConstant<IsLower_v<MT1> && IsLower_v<MT2> && IsLower_v<MT3>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2, typename MT3>
struct YieldsStrictlyLower<MAC, MT1, MT2, MT3>
	: public BoolConstant<IsStrictlyLower_v<MT1> && IsStrictlyLower_v<MT2> && IsStrictlyLower_v<MT3>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2, typename MT3>
struct YieldsUpper<MAC, MT1, MT2, MT3> : public BoolConstant<IsUpper_v<MT1> && IsUpper_v<MT2> && IsUpper_v<MT3>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2, typename MT3>
struct YieldsStrictlyUpper<MAC, MT1, MT2, MT3>
	: public BoolConstant<IsStrictlyUpper_v<MT1> && IsStrictlyUpper_v<MT2> && IsStrictlyUpper_v<MT3>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSZERO SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2, typename T3>
struct YieldsZero<MAC, T1, T2, T3> : public BoolConstant<IsZero_v<T1> && IsZero_v<T2> && IsZero_v<T3>> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
