// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_HYPOT_H
#define METRIC_NUMERIC_MATH_FUNCTORS_HYPOT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/constraints/SIMDPack.h>
#include <metric/numeric/math/shims/Hypot.h>
#include <metric/numeric/math/simd/Hypot.h>
#include <metric/numeric/math/typetraits/HasSIMDHypot.h>
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
/*!\brief Generic wrapper for the hypot() function.
// \ingroup functors
*/
struct Hypot {
	//**********************************************************************************************
	/*!\brief Returns the result of the hypot() function for the given objects/values.
	//
	// \param a The left-hand side object/value.
	// \param b The right-hand side object/value.
	// \return The result of the hypot() function for the given objects/values.
	*/
	template <typename T1, typename T2>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T1 &&a, T2 &&b) const
	{
		return hypot(std::forward<T1>(a), std::forward<T2>(b));
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data types \a T1 and \a T2.
	//
	// \return \a true in case SIMD is enabled for the data types \a T1 and \a T2, \a false if not.
	*/
	template <typename T1, typename T2> static constexpr bool simdEnabled() { return HasSIMDHypot_v<T1, T2>; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the operation supports padding, i.e. whether it can deal with zeros.
	//
	// \return \a true in case padding is supported, \a false if not.
	*/
	static constexpr bool paddingEnabled() { return true; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the hypot() function for the given SIMD vectors.
	//
	// \param a The left-hand side SIMD vector.
	// \param b The right-hand side SIMD vector.
	// \return The result of the hypot() function for the given SIMD vectors.
	*/
	template <typename T1, typename T2> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(const T1 &a, const T2 &b) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T1);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T2);
		return hypot(a, b);
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
struct YieldsUniform<Hypot, T1, T2> : public BoolConstant<IsUniform_v<T1> && IsUniform_v<T2>> {};
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
struct YieldsSymmetric<Hypot, MT1, MT2> : public BoolConstant<IsSymmetric_v<MT1> && IsSymmetric_v<MT2>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2>
struct YieldsLower<Hypot, MT1, MT2> : public BoolConstant<IsLower_v<MT1> && IsLower_v<MT2>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2>
struct YieldsStrictlyLower<Hypot, MT1, MT2> : public BoolConstant<IsStrictlyLower_v<MT1> && IsStrictlyLower_v<MT2>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2>
struct YieldsUpper<Hypot, MT1, MT2> : public BoolConstant<IsUpper_v<MT1> && IsUpper_v<MT2>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2>
struct YieldsStrictlyUpper<Hypot, MT1, MT2> : public BoolConstant<IsStrictlyUpper_v<MT1> && IsStrictlyUpper_v<MT2>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSZERO SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct YieldsZero<Hypot, T1, T2> : public BoolConstant<IsZero_v<T1> && IsZero_v<T2>> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
