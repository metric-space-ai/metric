// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_LPNORM_H
#define METRIC_NUMERIC_MATH_FUNCTORS_LPNORM_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/HostDevice.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/StaticAssert.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for the lpNorm() function.
// \ingroup functors
*/
template <size_t... P> // Compile time norm parameter
struct LpNorm {
	//**********************************************************************************************
	/*!\brief Calls the lpNorm() function with the given object/value.
	//
	// \param a The given object/value.
	// \return The Lp norm of the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T &&a) const
	{
		METRIC_NUMERIC_STATIC_ASSERT_MSG(sizeof...(P) == 1UL, "Missing norm parameter detected");
		return lpNorm(std::forward<T>(a), P...);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Calls the lpNorm() function with the given object/value.
	//
	// \param a The given object/value.
	// \param p The runtime norm parameter.
	// \return The Lp norm of the given object/value.
	*/
	template <typename T, typename ST>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(T &&a, ST p) const
	{
		METRIC_NUMERIC_STATIC_ASSERT_MSG(sizeof...(P) == 0UL, "Over-specified norm parameter detected");
		return lpNorm(std::forward<T>(a), p);
	}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  CLASS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of LpNorm class template for the L1 norm.
// \ingroup functors
*/
template <> struct LpNorm<1UL> {
	//**********************************************************************************************
	/*!\brief Calls the lpNorm() function with the given object/value.
	//
	// \param a The given object/value.
	// \return The Lp norm of the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(const T &a) const
	{
		return l1Norm(a);
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of LpNorm class template for the L2 norm.
// \ingroup functors
*/
template <> struct LpNorm<2UL> {
	//**********************************************************************************************
	/*!\brief Calls the lpNorm() function with the given object/value.
	//
	// \param a The given object/value.
	// \return The Lp norm of the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(const T &a) const
	{
		return l2Norm(a);
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of LpNorm class template for the L3 norm.
// \ingroup functors
*/
template <> struct LpNorm<3UL> {
	//**********************************************************************************************
	/*!\brief Calls the lpNorm() function with the given object/value.
	//
	// \param a The given object/value.
	// \return The Lp norm of the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(const T &a) const
	{
		return l3Norm(a);
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of LpNorm class template for the L4 norm.
// \ingroup functors
*/
template <> struct LpNorm<4UL> {
	//**********************************************************************************************
	/*!\brief Calls the lpNorm() function with the given object/value.
	//
	// \param a The given object/value.
	// \return The Lp norm of the given object/value.
	*/
	template <typename T>
	METRIC_NUMERIC_ALWAYS_INLINE METRIC_NUMERIC_DEVICE_CALLABLE decltype(auto) operator()(const T &a) const
	{
		return l4Norm(a);
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
