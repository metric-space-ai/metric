// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_IMAG_H
#define METRIC_NUMERIC_MATH_FUNCTORS_IMAG_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/shims/Imaginary.h>
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
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for the imag() function.
// \ingroup functors
*/
struct Imag {
	//**********************************************************************************************
	/*!\brief Returns the result of the imag() function for the given object/value.
	//
	// \param a The given object/value.
	// \return The result of the imag() function for the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T &&a) const
	{
		return imag(std::forward<T>(a));
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
template <typename T> struct YieldsUniform<Imag, T> : public IsUniform<T> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsSymmetric<Imag, MT> : public IsSymmetric<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsLower<Imag, MT> : public IsLower<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsStrictlyLower<Imag, MT> : public IsStrictlyLower<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsUpper<Imag, MT> : public IsUpper<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT> struct YieldsStrictlyUpper<Imag, MT> : public IsStrictlyUpper<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSZERO SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> struct YieldsZero<Imag, T> : public IsZero<T> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
