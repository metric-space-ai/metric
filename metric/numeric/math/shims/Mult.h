// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_MULT_H
#define METRIC_NUMERIC_MATH_SHIMS_MULT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  MULT SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Multiplication of the two given arguments.
// \ingroup math_shims
//
// \param lhs The left-hand side operand for the multiplication.
// \param rhs The right-hand side operand for the multiplication.
// \return The product of the two operands.
//
// The \a mult() shim represents an abstract interface for the multiplication of two operands.
*/
template <typename T1, typename T2>
METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) mult(T1 &&lhs, T2 &&rhs) noexcept(noexcept(lhs * rhs))
{
	return std::forward<T1>(lhs) * std::forward<T2>(rhs);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
