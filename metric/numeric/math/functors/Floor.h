// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_FLOOR_H
#define METRIC_NUMERIC_MATH_FUNCTORS_FLOOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/constraints/SIMDPack.h>
#include <metric/numeric/math/shims/Floor.h>
#include <metric/numeric/math/simd/Floor.h>
#include <metric/numeric/math/typetraits/HasSIMDFloor.h>
#include <metric/numeric/math/typetraits/IsLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
#include <metric/numeric/math/typetraits/IsUniLower.h>
#include <metric/numeric/math/typetraits/IsUniUpper.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
#include <metric/numeric/math/typetraits/IsUpper.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/math/typetraits/YieldsLower.h>
#include <metric/numeric/math/typetraits/YieldsStrictlyLower.h>
#include <metric/numeric/math/typetraits/YieldsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/YieldsSymmetric.h>
#include <metric/numeric/math/typetraits/YieldsUniLower.h>
#include <metric/numeric/math/typetraits/YieldsUniUpper.h>
#include <metric/numeric/math/typetraits/YieldsUniform.h>
#include <metric/numeric/math/typetraits/YieldsUpper.h>
#include <metric/numeric/math/typetraits/YieldsZero.h>
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
/*!\brief Generic wrapper for the floor() function.
// \ingroup functors
*/
struct Floor {
	//**********************************************************************************************
	/*!\brief Returns the result of the floor() function for the given object/value.
	//
	// \param a The given object/value.
	// \return The result of the floor() function for the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T &&a) const
	{
		return floor(std::forward<T>(a));
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data type \a T.
	//
	// \return \a true in case SIMD is enabled for the data type \a T, \a false if not.
	*/
	template <typename T> static constexpr bool simdEnabled() { return HasSIMDFloor_v<T>; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the operation supports padding, i.e. whether it can deal with zeros.
	//
	// \return \a true in case padding is supported, \a false if not.
	*/
	static constexpr bool paddingEnabled() { return true; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the floor() function for the given SIMD vector.
	//
	// \param a The given SIMD vector.
	// \return The result of the floor() function for the given SIMD vector.
	*/
	template <typename T> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(const T &a) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SIMD_PACK(T);
		return floor(a);
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
template <typename T> struct YieldsUniform<Floor, T> : public IsUniform<T> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsSymmetric<Floor, MT> : public IsSymmetric<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsLower<Floor, MT> : public IsLower<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUNILOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsUniLower<Floor, MT> : public IsUniLower<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsStrictlyLower<Floor, MT> : public IsStrictlyLower<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsUpper<Floor, MT> : public IsUpper<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUNIUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsUniUpper<Floor, MT> : public IsUniUpper<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsStrictlyUpper<Floor, MT> : public IsStrictlyUpper<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSZERO SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> struct YieldsZero<Floor, T> : public IsZero<T> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
