// Runtime metric-law coverage for the affine-invariant Riemannian distance
// (AIRM) on the SPD images of two equal-size finite metric spaces.
//
// metric_discovery_smoke.cpp only static_asserts the *declared* metric_traits
// law for RiemannianDistance; the runtime evidence that the law actually holds
// used to live solely in tests/distance_tests/riemannian_test.cpp, which is NOT
// part of the active core gate (METRIC_BUILD_TESTS=OFF, stale includes). This
// smoke promotes that runtime coverage into the active gate so the AIRM
// operand-encoding fix (both operands built as the same regularized-Laplacian
// SPD matrix) cannot silently regress.
//
// History: a previous version encoded one operand as the graph Laplacian
// (off-diagonal -d) and the other as diag(degree)+adjacency (off-diagonal +d),
// which made the generalized eigenproblem asymmetric and broke BOTH symmetry
// (d(X,Y) != d(Y,X)) and identity of indiscernibles (d(X,X) != 0). The fix in
// Riemannian.cpp restores both. This test pins symmetry, self-distance zero,
// non-negativity, finiteness, the triangle inequality, and the domain guard.

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include <metric/metric/catalog/space/Riemannian.hpp>

using Vec = std::vector<double>;
using Space = std::vector<Vec>;

namespace {

auto rd_value(const Space &a, const Space &b) -> double
{
	mtrc::RiemannianDistance<void, mtrc::Euclidean<double>> rd;
	return rd(a, b);
}

} // namespace

int main()
{
	mtrc::RiemannianDistance<void, mtrc::Euclidean<double>> rd;

	// Direct SPD-matrix value oracle (matches tests/distance_tests/riemannian_test.cpp):
	// AIRM geodesic distance between two fixed upper-triangular matrices.
	mtrc::numeric::DynamicMatrix<double> A{{1, 2, -3}, {0, -2, 4}, {0, 0, 1}};
	mtrc::numeric::DynamicMatrix<double> B{{2, 0, -2}, {0, 1, 1}, {0, 0, 1}};
	assert(std::abs(rd.matDistance(A, B) - 0.6931471805599455) < 1e-12);

	// Three equal-size finite metric spaces (each 4 records in R^2).
	const Space ds1{{0, 1}, {0, 0}, {1, 1}, {1, 0}};
	const Space ds2{{0, 0}, {1, 1}, {2, 2}, {2, 1}};
	const Space ds3{{0, 2}, {1, 0}, {2, 2}, {0, 1}};

	// Regression value of the fixed (symmetric) operand encoding.
	const double d12 = rd_value(ds1, ds2);
	assert(std::abs(d12 - 0.2038157977919085) < 1e-9);

	// Non-negativity and finiteness on every pair.
	const Space *spaces[] = {&ds1, &ds2, &ds3};
	for (const auto *x : spaces) {
		for (const auto *y : spaces) {
			const double d = rd_value(*x, *y);
			assert(std::isfinite(d));
			assert(d >= -1e-12);
		}
	}

	// Identity of indiscernibles: d(X, X) == 0.
	assert(std::abs(rd_value(ds1, ds1)) < 1e-9);
	assert(std::abs(rd_value(ds2, ds2)) < 1e-9);
	assert(std::abs(rd_value(ds3, ds3)) < 1e-9);

	// Symmetry: d(X, Y) == d(Y, X) (the defect that the operand-encoding fix repaired).
	assert(std::abs(rd_value(ds1, ds2) - rd_value(ds2, ds1)) < 1e-9);
	assert(std::abs(rd_value(ds1, ds3) - rd_value(ds3, ds1)) < 1e-9);
	assert(std::abs(rd_value(ds2, ds3) - rd_value(ds3, ds2)) < 1e-9);

	// Triangle inequality across all ordered triples of the three spaces.
	for (const auto *x : spaces) {
		for (const auto *y : spaces) {
			for (const auto *z : spaces) {
				const double dxy = rd_value(*x, *y);
				const double dxz = rd_value(*x, *z);
				const double dzy = rd_value(*z, *y);
				assert(dxy <= dxz + dzy + 1e-9);
			}
		}
	}

	// Domain guard: equal-size, non-empty finite spaces are required.
	bool threw_on_size_mismatch = false;
	try {
		(void)rd(ds1, Space{{0, 0}});
	} catch (const std::invalid_argument &) {
		threw_on_size_mismatch = true;
	}
	assert(threw_on_size_mismatch);

	bool threw_on_empty = false;
	try {
		(void)rd(Space{}, Space{});
	} catch (const std::invalid_argument &) {
		threw_on_empty = true;
	}
	assert(threw_on_empty);

	return 0;
}
