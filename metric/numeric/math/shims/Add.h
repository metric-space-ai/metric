// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ADD_H
#define METRIC_NUMERIC_MATH_SHIMS_ADD_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  ADD SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Addition of the two given arguments.
// \ingroup math_shims
//
// \param lhs The left-hand side operand for the addition.
// \param rhs The right-hand side operand for the addition.
// \return The sum of the two operands.
//
// The \a add() shim represents an abstract interface for the addition of two operands.
*/
template <typename T1, typename T2>
METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) add(T1 &&lhs, T2 &&rhs) noexcept(noexcept(lhs + rhs))
{
	return std::forward<T1>(lhs) + std::forward<T2>(rhs);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
