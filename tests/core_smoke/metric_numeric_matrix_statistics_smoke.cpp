#include <cassert>
#include <cmath>
#include <stdexcept>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

namespace {

using mtrc::test::close_to;

template <typename Vector>
auto assert_vector_values(const Vector &vector, std::initializer_list<double> expected) -> void
{
	assert(vector.size() == expected.size());

	std::size_t index = 0;
	for (const double value : expected) {
		assert(close_to(vector[index], value));
		++index;
	}
}

template <typename Function> auto assert_invalid_argument(Function function) -> void
{
	bool rejected = false;
	try {
		function();
	} catch (const std::invalid_argument &) {
		rejected = true;
	}

	assert(rejected);
}

} // namespace

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using SparseMatrix = mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor>;

	Matrix values(3, 3);
	values(0, 0) = 1.0;
	values(0, 1) = 3.0;
	values(0, 2) = 2.0;
	values(1, 0) = 2.0;
	values(1, 1) = 6.0;
	values(1, 2) = 4.0;
	values(2, 0) = 9.0;
	values(2, 1) = 6.0;
	values(2, 2) = 3.0;

	const auto row_variance = mtrc::numeric::var<mtrc::numeric::rowwise>(values);
	assert_vector_values(row_variance, {1.0, 4.0, 9.0});

	const auto column_variance = mtrc::numeric::var<mtrc::numeric::columnwise>(values);
	assert_vector_values(column_variance, {19.0, 3.0, 1.0});

	const auto row_stddev = mtrc::numeric::stddev<mtrc::numeric::rowwise>(values);
	assert_vector_values(row_stddev, {1.0, 2.0, 3.0});

	const auto column_stddev = mtrc::numeric::stddev<mtrc::numeric::columnwise>(values);
	assert_vector_values(column_stddev, {std::sqrt(19.0), std::sqrt(3.0), 1.0});

	SparseMatrix sparse(3, 3);
	sparse.reserve(9);
	sparse.append(0, 0, 1.0);
	sparse.append(0, 1, 3.0);
	sparse.append(0, 2, 2.0);
	sparse.finalize(0);
	sparse.append(1, 0, 2.0);
	sparse.append(1, 1, 6.0);
	sparse.append(1, 2, 4.0);
	sparse.finalize(1);
	sparse.append(2, 0, 9.0);
	sparse.append(2, 1, 6.0);
	sparse.append(2, 2, 3.0);
	sparse.finalize(2);

	assert_vector_values(mtrc::numeric::var<mtrc::numeric::rowwise>(sparse), {1.0, 4.0, 9.0});
	assert_vector_values(mtrc::numeric::var<mtrc::numeric::columnwise>(sparse), {19.0, 3.0, 1.0});
	assert_vector_values(mtrc::numeric::stddev<mtrc::numeric::rowwise>(sparse), {1.0, 2.0, 3.0});
	assert_vector_values(mtrc::numeric::stddev<mtrc::numeric::columnwise>(sparse),
						 {std::sqrt(19.0), std::sqrt(3.0), 1.0});

	Matrix single_column(3, 1);
	single_column(0, 0) = 1.0;
	single_column(1, 0) = 2.0;
	single_column(2, 0) = 3.0;
	assert_invalid_argument([&single_column] { (void)mtrc::numeric::var<mtrc::numeric::rowwise>(single_column); });

	Matrix single_row(1, 3);
	single_row(0, 0) = 1.0;
	single_row(0, 1) = 2.0;
	single_row(0, 2) = 3.0;
	assert_invalid_argument([&single_row] { (void)mtrc::numeric::stddev<mtrc::numeric::columnwise>(single_row); });

	return 0;
}
