#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using SparseMatrix = mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor>;
	using SparseVector = mtrc::numeric::CompressedVector<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::close_to;

	Vector lhs(3);
	lhs[0] = 2.0;
	lhs[1] = 1.0;
	lhs[2] = -7.0;

	Vector rhs(3);
	rhs[0] = 5.0;
	rhs[1] = -3.0;
	rhs[2] = 5.0;

	const auto difference = lhs - rhs;
	assert(close_to(mtrc::numeric::norm(difference), 13.0));
	assert(close_to(mtrc::numeric::norm(lhs - rhs), 13.0));
	assert(close_to(mtrc::numeric::sqrNorm(difference), 169.0));
	assert(close_to(mtrc::numeric::l1Norm(difference), 19.0));
	assert(close_to(mtrc::numeric::l2Norm(difference), 13.0));
	assert(close_to(mtrc::numeric::l3Norm(difference), std::cbrt(1819.0)));
	assert(close_to(mtrc::numeric::l4Norm(difference), std::sqrt(std::sqrt(21073.0))));
	assert(close_to(mtrc::numeric::lpNorm(difference, 3.0), std::cbrt(1819.0)));
	assert(close_to(mtrc::numeric::lpNorm<3UL>(difference), std::cbrt(1819.0)));
	assert(close_to(mtrc::numeric::linfNorm(difference), 12.0));
	assert(close_to(mtrc::numeric::min(difference), -12.0));
	assert(close_to(mtrc::numeric::max(difference), 4.0));

	SparseVector sparse_difference(5);
	sparse_difference.reserve(3);
	sparse_difference.append(0, -3.0);
	sparse_difference.append(2, 4.0);
	sparse_difference.set(4, -12.0);

	assert(close_to(mtrc::numeric::norm(sparse_difference), 13.0));
	assert(close_to(mtrc::numeric::sqrNorm(sparse_difference), 169.0));
	assert(close_to(mtrc::numeric::l1Norm(sparse_difference), 19.0));
	assert(close_to(mtrc::numeric::linfNorm(sparse_difference), 12.0));
	assert(close_to(mtrc::numeric::lpNorm<3UL>(sparse_difference), std::cbrt(1819.0)));

	Matrix values(2, 3);
	values(0, 0) = 1.0;
	values(0, 1) = -2.0;
	values(0, 2) = 3.0;
	values(1, 0) = -4.0;
	values(1, 1) = 0.0;
	values(1, 2) = 5.0;

	assert(close_to(mtrc::numeric::norm(values), std::sqrt(55.0)));
	assert(close_to(mtrc::numeric::sqrNorm(values), 55.0));
	assert(close_to(mtrc::numeric::l1Norm(values), 15.0));
	assert(close_to(mtrc::numeric::l2Norm(values), std::sqrt(55.0)));
	assert(close_to(mtrc::numeric::l3Norm(values), std::cbrt(225.0)));
	assert(close_to(mtrc::numeric::l4Norm(values), std::sqrt(std::sqrt(979.0))));
	assert(close_to(mtrc::numeric::lpNorm(values, 3.0), std::cbrt(225.0)));
	assert(close_to(mtrc::numeric::lpNorm<3UL>(values), std::cbrt(225.0)));
	assert(close_to(mtrc::numeric::linfNorm(values), 5.0));
	assert(close_to(mtrc::numeric::min(values), -4.0));
	assert(close_to(mtrc::numeric::max(values), 5.0));

	SparseMatrix sparse_values(2, 3);
	sparse_values.reserve(5);
	sparse_values.append(0, 0, 1.0);
	sparse_values.append(0, 1, -2.0);
	sparse_values.append(0, 2, 3.0);
	sparse_values.finalize(0);
	sparse_values.append(1, 0, -4.0);
	sparse_values.append(1, 2, 5.0);
	sparse_values.finalize(1);

	assert(close_to(mtrc::numeric::norm(sparse_values), std::sqrt(55.0)));
	assert(close_to(mtrc::numeric::sqrNorm(sparse_values), 55.0));
	assert(close_to(mtrc::numeric::l1Norm(sparse_values), 15.0));
	assert(close_to(mtrc::numeric::linfNorm(sparse_values), 5.0));
	assert(close_to(mtrc::numeric::lpNorm<3UL>(sparse_values), std::cbrt(225.0)));

	return 0;
}
