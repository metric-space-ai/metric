// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_EVALUATE_H
#define METRIC_NUMERIC_MATH_SHIMS_EVALUATE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/HasCompositeType.h>
#include <metric/numeric/math/typetraits/IsProxy.h>
#include <metric/numeric/util/EnableIf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  EVALUATE SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Formal evaluation of the given argument.
// \ingroup math_shims
//
// \param a The value/object to be evaluated.
// \return The evaluated value/object.
//
// The \a evaluate shim represents an abstract interface for enforcing the evaluation of a given
// value/object of any data type and deducing the correct result type of an operation. For data
// types that don't require an evaluation, as for instance built-in or complex data types, the
// default behavior is not changed.
*/
template <typename T, EnableIf_t<!HasCompositeType_v<T> && !IsProxy_v<T>> * = nullptr>
constexpr T evaluate(const T &a) noexcept
{
	return a;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
