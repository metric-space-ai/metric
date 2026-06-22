// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ALGORITHMS_MINMAX_H
#define METRIC_NUMERIC_UTIL_ALGORITHMS_MINMAX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/CommonType.h>
#include <metric/numeric/util/typetraits/HasLessThan.h>
#include <metric/numeric/util/typetraits/IsReference.h>
#include <metric/numeric/util/typetraits/IsSame.h>
#include <metric/numeric/util/typetraits/IsSigned.h>
#include <metric/numeric/util/typetraits/IsUnsigned.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  MINMAX ALGORITHMS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Minmax function for two values/objects.
// \ingroup algorithms
//
// \param a The first value/object.
// \param b The second value/object.
// \return A pair of the minimum and maximum of the two values/objects.
//
// This function determines the minimum and maximium of the two given values/objects by means of
// a less-than comparison. The return type of the function is determined by the data types of the
// given arguments.
*/
template <typename T1, typename T2, typename R1 = RemoveCVRef_t<T1>, typename R2 = RemoveCVRef_t<T2>,
		  EnableIf_t<HasLessThan_v<R2, R1> && !(IsSigned_v<T1> && IsUnsigned_v<T2>) &&
					 !(IsUnsigned_v<T1> && IsSigned_v<T2>)> * = nullptr>
constexpr decltype(auto) minmax(T1 &&a, T2 &&b)
{
	using RT = If_t<IsReference_v<T1> && IsReference_v<T2> && IsSame_v<R1, R2>, const R1 &, CommonType_t<R1, R2>>;

	if (b < a)
		return std::pair<RT, RT>(std::forward<T2>(b), std::forward<T1>(a));
	else
		return std::pair<RT, RT>(std::forward<T1>(a), std::forward<T2>(b));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
