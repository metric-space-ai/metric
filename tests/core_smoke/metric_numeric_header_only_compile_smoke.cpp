#include <cassert>
#include <cstddef>
#include <tuple>
#include <vector>

#include <metric/numeric.hpp>

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;

	Vector input(3);
	input[0] = 1.0;
	input[1] = -2.0;
	input[2] = 4.0;

	Matrix transform(3, 3, 0.0);
	transform(0, 0) = 2.0;
	transform(1, 1) = 3.0;
	transform(2, 2) = 4.0;

	const Vector transformed = transform * input;
	assert(transformed[0] == 2.0);
	assert(transformed[1] == -6.0);
	assert(transformed[2] == 16.0);

	mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor> sparse(3, 3);
	sparse.reserve(5);
	sparse.append(0, 0, 2.0);
	sparse.append(0, 1, -1.0);
	sparse.finalize(0);
	sparse.append(1, 1, 2.0);
	sparse.append(1, 2, -1.0);
	sparse.finalize(1);
	sparse.append(2, 2, 2.0);
	sparse.finalize(2);

	const Vector sparse_product = sparse * input;
	assert(sparse_product[0] == 4.0);
	assert(sparse_product[1] == -8.0);
	assert(sparse_product[2] == 8.0);

	using Edge = std::tuple<std::size_t, std::size_t, double>;
	const auto shortest_paths =
		mtrc::numeric::graph_shortest_path_distances(3, std::vector<Edge>{{0, 1, 1.5}, {1, 2, 2.0}}, false);
	assert(shortest_paths[0][2] == 3.5);

	mtrc::numeric::DynamicMatrix<double> small_system(2, 2);
	small_system(0, 0) = 4.0;
	small_system(0, 1) = 1.0;
	small_system(1, 0) = 2.0;
	small_system(1, 1) = 3.0;
	mtrc::numeric::invert2x2<mtrc::numeric::asGeneral>(small_system);
	assert(mtrc::numeric::abs(small_system(0, 0) - 0.3) < 1e-10);
	assert(mtrc::numeric::abs(small_system(1, 1) - 0.4) < 1e-10);

	const auto original_seed = mtrc::numeric::getSeed();
	mtrc::numeric::setSeed(314159U);
	const double random_value = mtrc::numeric::rand<double>(-1.0, 1.0);
	assert(random_value >= -1.0);
	assert(random_value <= 1.0);
	mtrc::numeric::setSeed(original_seed);

	return 0;
}
