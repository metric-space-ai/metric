// Exercises promoted mtrc::numeric contracts through the curated Level-2 facade
// headers ONLY (not the metric/numeric.hpp umbrella). This proves each facade
// re-exports the promoted public surface for its area and that the facades
// compose without clashing.
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <sstream>
#include <tuple>
#include <vector>

#include <metric/numeric/scalar.hpp>
#include <metric/numeric/vector.hpp>
#include <metric/numeric/matrix.hpp>
#include <metric/numeric/sparse.hpp>
#include <metric/numeric/linear_algebra.hpp>
#include <metric/numeric/graph.hpp>
#include <metric/numeric/random.hpp>
#include <metric/numeric/parallel.hpp>
#include <metric/numeric/io.hpp>

#include "numeric_test_helpers.hpp"

namespace mn = mtrc::numeric;

int main()
{
	using mtrc::test::close_to;

	// scalar.hpp — element shims, complex helpers, tolerance comparison, and the
	// numeric accuracy/epsilon/infinity constants (promoted but previously untested).
	assert(close_to(mn::sqrt(16.0), 4.0));
	assert(close_to(mn::abs(-2.5), 2.5));
	assert(close_to(mn::pow(2.0, 3.0), 8.0));
	const auto z = mn::complex<double>(3.0, -4.0);
	assert(close_to(mn::real(z), 3.0));
	assert(close_to(mn::imag(z), -4.0));
	assert(mn::equal(1.0, 1.0));
	assert(!mn::equal(1.0, 2.0));
	const double tolerance = mn::accuracy;
	const double machine_epsilon = mn::epsilon;
	const double largest_value = mn::inf;
	assert(tolerance >= 0.0);
	assert(machine_epsilon > 0.0);
	// inf is the per-type largest representable value sentinel (not IEEE infinity).
	assert(largest_value == std::numeric_limits<double>::max());

	// vector.hpp — dense vector container plus the construction-tag vocabulary
	// (transpose flag, alignment, and padding flags).
	mn::DynamicVector<double> v(3);
	v[0] = 1.0;
	v[1] = -2.0;
	v[2] = 4.0;
	assert(v.size() == 3);
	const mn::StaticVector<double, 3UL, mn::rowVector> row{1.0, 2.0, 3.0};
	assert(close_to(row[2], 3.0));
	constexpr bool alignment_tags_distinct = (mn::aligned != mn::unaligned);
	constexpr bool padding_tags_distinct = (mn::padded != mn::unpadded);
	static_assert(alignment_tags_distinct, "alignment flags must be distinct");
	static_assert(padding_tags_distinct, "padding flags must be distinct");

	// matrix.hpp — dense matrix, matrix-vector product, reduction, primitives, adaptor.
	mn::DynamicMatrix<double> diagonal_scale(3, 3, 0.0);
	diagonal_scale(0, 0) = 2.0;
	diagonal_scale(1, 1) = 3.0;
	diagonal_scale(2, 2) = 4.0;
	const mn::DynamicVector<double> scaled = diagonal_scale * v;
	assert(close_to(scaled[0], 2.0));
	assert(close_to(scaled[1], -6.0));
	assert(close_to(scaled[2], 16.0));
	assert(close_to(mn::sum(diagonal_scale), 9.0));
	const auto row_totals = mn::row_sums(diagonal_scale); // MatrixPrimitives helper.
	assert(close_to(row_totals[2], 4.0));
	mn::LowerMatrix<mn::DynamicMatrix<double>> lower(2, 2);
	lower(1, 0) = 5.0;
	assert(lower.isIntact());

	// sparse.hpp — compressed matrix product.
	mn::CompressedMatrix<double, mn::rowMajor> sparse(3, 3);
	sparse.reserve(5);
	sparse.append(0, 0, 2.0);
	sparse.append(0, 1, -1.0);
	sparse.finalize(0);
	sparse.append(1, 1, 2.0);
	sparse.append(1, 2, -1.0);
	sparse.finalize(1);
	sparse.append(2, 2, 2.0);
	sparse.finalize(2);
	const mn::DynamicVector<double> sparse_product = sparse * v;
	assert(close_to(sparse_product[0], 4.0));
	assert(close_to(sparse_product[1], -8.0));
	assert(close_to(sparse_product[2], 8.0));

	// linear_algebra.hpp — explicit small inverse (link-free path).
	mn::DynamicMatrix<double> system(2, 2);
	system(0, 0) = 4.0;
	system(0, 1) = 1.0;
	system(1, 0) = 2.0;
	system(1, 1) = 3.0;
	mn::invert2x2<mn::asGeneral>(system);
	assert(close_to(system(0, 0), 0.3));
	assert(close_to(system(1, 1), 0.4));

	// graph.hpp — all-pairs shortest paths over raw index edges.
	using Edge = std::tuple<std::size_t, std::size_t, double>;
	const auto shortest_paths =
		mn::graph_shortest_path_distances(3, std::vector<Edge>{{0, 1, 1.5}, {1, 2, 2.0}}, false);
	assert(close_to(shortest_paths[0][2], 3.5));

	// random.hpp — seeded reproducibility.
	const auto original_seed = mn::getSeed();
	mn::setSeed(2718281U);
	const double first = mn::rand<double>(-1.0, 1.0);
	mn::setSeed(2718281U);
	assert(close_to(mn::rand<double>(-1.0, 1.0), first));
	mn::setSeed(original_seed);

	// io.hpp — Archive round-trip through the serialization facade.
	std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
	{
		mn::Archive<std::stringstream> writer(stream);
		writer << diagonal_scale << v;
		assert(writer);
	}
	stream.clear();
	stream.seekg(0);
	mn::DynamicMatrix<double> loaded_matrix;
	mn::DynamicVector<double> loaded_vector;
	{
		mn::Archive<std::stringstream> reader(stream);
		reader >> loaded_matrix >> loaded_vector;
		assert(reader);
	}
	assert(loaded_matrix.rows() == diagonal_scale.rows());
	assert(loaded_vector.size() == v.size());
	assert(close_to(loaded_vector[2], 4.0));

	// parallel.hpp is included above; the default build is single-threaded and the
	// facade introduces no threading dependency, so inclusion is the contract.

	return 0;
}
