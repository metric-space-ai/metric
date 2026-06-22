#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>

namespace mtrc::test {

inline auto close_to(double lhs, double rhs, double tolerance = 1e-10) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
}

inline auto assert_close(double lhs, double rhs, double tolerance = 1e-10) -> void
{
	assert(close_to(lhs, rhs, tolerance));
}

template <typename LeftVector, typename RightVector>
auto assert_vector_close(const LeftVector &lhs, const RightVector &rhs, double tolerance = 1e-10) -> void
{
	assert(lhs.size() == rhs.size());

	for (std::size_t index = 0; index < lhs.size(); ++index) {
		assert_close(lhs[index], rhs[index], tolerance);
	}
}

template <typename LeftMatrix, typename RightMatrix>
auto assert_matrix_close(const LeftMatrix &lhs, const RightMatrix &rhs, double tolerance = 1e-10) -> void
{
	assert(lhs.rows() == rhs.rows());
	assert(lhs.columns() == rhs.columns());

	for (std::size_t row = 0; row < lhs.rows(); ++row) {
		for (std::size_t column = 0; column < lhs.columns(); ++column) {
			assert_close(lhs(row, column), rhs(row, column), tolerance);
		}
	}
}

} // namespace mtrc::test
