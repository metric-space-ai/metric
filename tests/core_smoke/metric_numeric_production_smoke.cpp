#include <cassert>
#include <chrono>
#include <cstddef>
#include <tuple>
#include <vector>

#include <metric/numeric.hpp>

#include "numeric_test_helpers.hpp"

namespace {

using Clock = std::chrono::steady_clock;

auto assert_elapsed_under(const Clock::time_point start, double seconds) -> void
{
	const auto elapsed = std::chrono::duration<double>(Clock::now() - start).count();
	assert(elapsed < seconds);
}

} // namespace

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::assert_matrix_close;
	using mtrc::test::assert_vector_close;
	using mtrc::test::close_to;

	const auto started = Clock::now();
	constexpr std::size_t size = 32;

	Matrix left(size, size);
	Matrix right(size, size);
	for (std::size_t row = 0; row < size; ++row) {
		for (std::size_t column = 0; column < size; ++column) {
			left(row, column) = 0.25 * static_cast<double>((row + 1) + (column % 5));
			right(row, column) = 0.125 * static_cast<double>((column + 2) - (row % 3));
		}
	}

	const Matrix product = left * right;
	auto expected_product_at = [&](std::size_t row, std::size_t column) {
		double total = 0.0;
		for (std::size_t index = 0; index < size; ++index) {
			total += left(row, index) * right(index, column);
		}
		return total;
	};
	assert(close_to(product(0, 0), expected_product_at(0, 0)));
	assert(close_to(product(17, 9), expected_product_at(17, 9)));
	assert(close_to(product(31, 31), expected_product_at(31, 31)));

	Vector values(size);
	Vector weights(size);
	for (std::size_t index = 0; index < size; ++index) {
		values[index] = static_cast<double>(index % 7) - 3.0;
		weights[index] = 1.0 + static_cast<double>(index) / static_cast<double>(size);
	}
	const double baseline_dot = mtrc::numeric::dot(values, weights);
	for (std::size_t iteration = 0; iteration < 128; ++iteration) {
		values += 0.001 * weights;
		values -= 0.001 * weights;
	}
	assert(close_to(mtrc::numeric::dot(values, weights), baseline_dot));
	assert(mtrc::numeric::l2Norm(weights) > 0.0);

	mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor> sparse(size, size);
	sparse.reserve((size * 3) - 2);
	for (std::size_t row = 0; row < size; ++row) {
		if (row > 0) {
			sparse.append(row, row - 1, -1.0);
		}
		sparse.append(row, row, 2.0);
		if (row + 1 < size) {
			sparse.append(row, row + 1, -1.0);
		}
		sparse.finalize(row);
	}
	const Vector sparse_product = sparse * weights;
	assert(close_to(sparse_product[0], (2.0 * weights[0]) - weights[1]));
	assert(close_to(sparse_product[17], -weights[16] + (2.0 * weights[17]) - weights[18]));
	assert(close_to(sparse_product[size - 1], -weights[size - 2] + (2.0 * weights[size - 1])));

	using Edge = std::tuple<std::size_t, std::size_t, double>;
	const auto shortest_paths = mtrc::numeric::graph_shortest_path_distances(
		5, std::vector<Edge>{{0, 1, 1.0}, {1, 2, 2.0}, {0, 3, 10.0}, {2, 3, 1.5}, {3, 4, 0.5}}, false);
	assert(close_to(shortest_paths[0][4], 5.0));
	assert(close_to(shortest_paths[4][0], 5.0));

	const auto original_seed = mtrc::numeric::getSeed();
	mtrc::numeric::setSeed(20260622U);
	Vector random_values(16);
	for (std::size_t index = 0; index < random_values.size(); ++index) {
		random_values[index] = mtrc::numeric::rand<double>(-2.0, 2.0);
		assert(random_values[index] >= -2.0);
		assert(random_values[index] <= 2.0);
	}
	mtrc::numeric::setSeed(20260622U);
	Vector repeated_random_values(16);
	for (std::size_t index = 0; index < repeated_random_values.size(); ++index) {
		repeated_random_values[index] = mtrc::numeric::rand<double>(-2.0, 2.0);
	}
	assert_vector_close(random_values, repeated_random_values);
	mtrc::numeric::setSeed(original_seed);

	Matrix small_system(2, 2);
	small_system(0, 0) = 4.0;
	small_system(0, 1) = 1.0;
	small_system(1, 0) = 2.0;
	small_system(1, 1) = 3.0;
	Matrix inverse_system = small_system;
	mtrc::numeric::invert2x2<mtrc::numeric::asGeneral>(inverse_system);
	Matrix identity(2, 2, 0.0);
	identity(0, 0) = 1.0;
	identity(1, 1) = 1.0;
	assert_matrix_close(small_system * inverse_system, identity);

	assert_elapsed_under(started, 10.0);
	return 0;
}
