#include <cassert>
#include <cmath>

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

} // namespace

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;

	Matrix values(2, 3);
	values(0, 0) = 1.0;
	values(0, 1) = 2.0;
	values(0, 2) = 3.0;
	values(1, 0) = 2.0;
	values(1, 1) = 0.0;
	values(1, 2) = -2.0;

	const auto row_sums = mtrc::numeric::sum<mtrc::numeric::rowwise>(values);
	assert_vector_values(row_sums, {6.0, 0.0});

	const auto column_sums = mtrc::numeric::sum<mtrc::numeric::columnwise>(values);
	assert_vector_values(column_sums, {3.0, 2.0, 1.0});

	const auto row_means = mtrc::numeric::mean<mtrc::numeric::rowwise>(values);
	assert_vector_values(row_means, {2.0, 0.0});

	const auto column_means = mtrc::numeric::mean<mtrc::numeric::columnwise>(values);
	assert_vector_values(column_means, {1.5, 1.0, 0.5});

	const auto row_products = mtrc::numeric::prod<mtrc::numeric::rowwise>(values);
	assert_vector_values(row_products, {6.0, 0.0});

	const auto column_products = mtrc::numeric::prod<mtrc::numeric::columnwise>(values);
	assert_vector_values(column_products, {2.0, 0.0, -6.0});

	assert(close_to(mtrc::numeric::prod(values), 0.0));

	const auto row_minimums = mtrc::numeric::min<mtrc::numeric::rowwise>(values);
	assert_vector_values(row_minimums, {1.0, -2.0});

	const auto column_maximums = mtrc::numeric::max<mtrc::numeric::columnwise>(values);
	assert_vector_values(column_maximums, {2.0, 2.0, 3.0});

	const Matrix probabilities = mtrc::numeric::softmax<mtrc::numeric::rowwise>(values);
	assert(probabilities.rows() == values.rows());
	assert(probabilities.columns() == values.columns());

	for (std::size_t row = 0; row < probabilities.rows(); ++row) {
		double probability_sum = 0.0;
		for (std::size_t column = 0; column < probabilities.columns(); ++column) {
			assert(probabilities(row, column) > 0.0);
			probability_sum += probabilities(row, column);
		}
		assert(close_to(probability_sum, 1.0));
	}

	const double first_row_normalizer = std::exp(1.0) + std::exp(2.0) + std::exp(3.0);
	assert(close_to(probabilities(0, 0), std::exp(1.0) / first_row_normalizer));
	assert(close_to(probabilities(0, 1), std::exp(2.0) / first_row_normalizer));
	assert(close_to(probabilities(0, 2), std::exp(3.0) / first_row_normalizer));

	const double second_row_normalizer = std::exp(2.0) + std::exp(0.0) + std::exp(-2.0);
	assert(close_to(probabilities(1, 0), std::exp(2.0) / second_row_normalizer));
	assert(close_to(probabilities(1, 1), std::exp(0.0) / second_row_normalizer));
	assert(close_to(probabilities(1, 2), std::exp(-2.0) / second_row_normalizer));

	return 0;
}
