// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ALGORITHMS_MIN_H
#define METRIC_NUMERIC_UTIL_ALGORITHMS_MIN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/HasLessThan.h>
#include <metric/numeric/util/typetraits/IsSigned.h>
#include <metric/numeric/util/typetraits/IsUnsigned.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>
#include <metric/numeric/util/typetraits/RemoveConst.h>
#include <metric/numeric/util/typetraits/RemoveRValueReference.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  MIN ALGORITHMS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Minimum function for two values/objects.
// \ingroup algorithms
//
// \param a The first value/object.
// \param b The second value/object.
// \return The minimum of the two values/objects.
//
// This function determines the minimum of the two given values by means of a less-than comparison.
// The return type of the function is determined by the data types of the given arguments.
*/
template <typename T1, typename T2, typename R1 = RemoveCVRef_t<T1>, typename R2 = RemoveCVRef_t<T2>,
		  EnableIf_t<HasLessThan_v<R2, R1> && !(IsSigned_v<R1> && IsUnsigned_v<R2>) &&
					 !(IsUnsigned_v<R1> && IsSigned_v<R2>)> * = nullptr>
constexpr decltype(auto) min(T1 &&a, T2 &&b)
{
	using Result = decltype(b < a ? std::forward<T2>(b) : std::forward<T1>(a));
	using Return = RemoveConst_t<RemoveRValueReference_t<Result>>;

	return static_cast<Return>(b < a ? std::forward<T2>(b) : std::forward<T1>(a));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Minimum function for three values/objects.
// \ingroup algorithms
//
// \param a The first value/object.
// \param b The second value/object.
// \param c The third value/object.
// \return The minimum of the given values/objects.
//
// This function returns the minimum of the given values/objects. The return type of the function
// is determined by the data types of the given arguments.
*/
template <typename T1, typename T2, typename T3> constexpr decltype(auto) min(T1 &&a, T2 &&b, T3 &&c)
{
	using mtrc::numeric::min;

	return min(min(std::forward<T1>(a), std::forward<T2>(b)), std::forward<T3>(c));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Minimum function for at least four values/objects.
// \ingroup algorithms
//
// \param a The first value/object.
// \param b The second value/object.
// \param c The third value/object.
// \param args The pack of additional values/objects.
// \return The minimum of the given values/objects.
//
// This function returns the minimum of the given values/objects. The return type of the function
// is determined by the data types of the given arguments.
*/
template <typename T1, typename T2, typename T3, typename... Ts>
constexpr decltype(auto) min(T1 &&a, T2 &&b, T3 &&c, Ts &&...args)
{
	using mtrc::numeric::min;

	return min(min(min(std::forward<T1>(a), std::forward<T2>(b)), std::forward<T3>(c)), std::forward<Ts>(args)...);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
