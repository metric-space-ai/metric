// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_SHIFTLV_H
#define METRIC_NUMERIC_MATH_FUNCTORS_SHIFTLV_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/constraints/SIMDPack.h>
#include <metric/numeric/math/simd/ShiftLV.h>
#include <metric/numeric/math/typetraits/HasSIMDShiftLV.h>
#include <metric/numeric/math/typetraits/IsLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
#include <metric/numeric/math/typetraits/IsUpper.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/math/typetraits/YieldsLower.h>
#include <metric/numeric/math/typetraits/YieldsStrictlyLower.h>
#include <metric/numeric/math/typetraits/YieldsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/YieldsSymmetric.h>
#include <metric/numeric/math/typetraits/YieldsUniform.h>
#include <metric/numeric/math/typetraits/YieldsUpper.h>
#include <metric/numeric/math/typetraits/YieldsZero.h>
#include <metric/numeric/system/Inline.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for the elementwise left-shift operation.
// \ingroup functors
*/
struct ShiftLV {
	//**********************************************************************************************
	/*!\brief Returns the result of the elementwise left-shift operation for the given objects/values.
	//
	// \param a The given object/value to be shifted.
	// \param b The number of bits to shift.
	// \return The result of the elementwise left-shift operation for the given objects/values.
	*/
	template <typename T1, typename T2> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) operator()(T1 &&a, T2 &&b) const
	{
		return std::forward<T1>(a) << std::forward<T2>(b);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data types \a T1 and \a T2.
	//
	// \return \a true in case SIMD is enabled for the data types \a T1 and \a T2, \a false if not.
	*/
	template <typename T1, typename T2> static constexpr bool simdEnabled() { return HasSIMDShiftLV_v<T1, T2>; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the operation supports padding, i.e. whether it can deal with zeros.
	//
	// \return \a true in case padding is supported, \a false if not.
	*/
	static constexpr bool paddingEnabled() { return true; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the elementwise left-shift operation for the given SIMD vector.
	//
	// \param a The SIMD vector to be shifted.
	// \param b The SIMD vector of bits to shift.
	// \return The result of the elementwise left-shift operation for the given SIMD vector.
	*/
	template <typename T1, typename T2> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(const T1 &a, const T2 &b) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T1);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T2);
		return a << b;
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
template <typename T1, typename T2>
struct YieldsUniform<ShiftLV, T1, T2> : public BoolConstant<IsUniform_v<T1> && IsUniform_v<T2>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2>
struct YieldsSymmetric<ShiftLV, MT1, MT2> : public BoolConstant<IsSymmetric_v<MT1> && IsSymmetric_v<MT2>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2> struct YieldsLower<ShiftLV, MT1, MT2> : public IsLower<MT1> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2> struct YieldsStrictlyLower<ShiftLV, MT1, MT2> : public IsStrictlyLower<MT1> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2> struct YieldsUpper<ShiftLV, MT1, MT2> : public IsUpper<MT1> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2> struct YieldsStrictlyUpper<ShiftLV, MT1, MT2> : public IsStrictlyUpper<MT1> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSZERO SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2> struct YieldsZero<ShiftLV, T1, T2> : public IsZero<T1> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
