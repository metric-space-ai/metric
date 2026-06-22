// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_DIV_H
#define METRIC_NUMERIC_MATH_SHIMS_DIV_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  DIV SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Division of the two given arguments.
// \ingroup math_shims
//
// \param lhs The left-hand side operand for the division.
// \param rhs The right-hand side operand for the division.
// \return The quotient of the two operands.
//
// The \a div() shim represents an abstract interface for the division of two operands.
*/
template <typename T1, typename T2>
METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) div(T1 &&lhs, T2 &&rhs) noexcept(noexcept(lhs / rhs))
{
	return std::forward<T1>(lhs) / std::forward<T2>(rhs);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
