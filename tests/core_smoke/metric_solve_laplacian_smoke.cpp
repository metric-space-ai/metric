#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include <metric/solve/laplacian/helper/ijvstruct.hpp>
#include <metric/solve/laplacian/solver.hpp>

#include "numeric_test_helpers.hpp"

namespace {

using SparseMatrix = mtrc::numeric::CompressedMatrix<double, mtrc::numeric::columnMajor>;
using Vector = mtrc::numeric::DynamicVector<double>;

auto make_spd_system() -> SparseMatrix
{
	SparseMatrix matrix(3, 3);
	matrix.reserve(7);

	matrix.append(0, 0, 4.0);
	matrix.append(1, 0, 1.0);
	matrix.finalize(0);

	matrix.append(0, 1, 1.0);
	matrix.append(1, 1, 3.0);
	matrix.append(2, 1, 1.0);
	matrix.finalize(1);

	matrix.append(1, 2, 1.0);
	matrix.append(2, 2, 2.0);
	matrix.finalize(2);

	return matrix;
}

// 2x2 symmetric positive definite system [[2,1],[1,2]] with a hand-checkable exact solution.
auto make_2x2_spd_system() -> SparseMatrix
{
	SparseMatrix matrix(2, 2);
	matrix.reserve(4);

	matrix.append(0, 0, 2.0);
	matrix.append(1, 0, 1.0);
	matrix.finalize(0);

	matrix.append(0, 1, 1.0);
	matrix.append(1, 1, 2.0);
	matrix.finalize(1);

	return matrix;
}

// Symmetric diagonally dominant M-matrix: path Laplacian on 3 nodes plus a boundary surplus at
// node 0, so adj()/extendMatrix() build a genuine (n+1)-node augmented system inside sddmWrapLap.
auto make_sddm_system() -> SparseMatrix
{
	SparseMatrix matrix(3, 3);
	matrix.reserve(7);

	matrix.append(0, 0, 2.0);
	matrix.append(1, 0, -1.0);
	matrix.finalize(0);

	matrix.append(0, 1, -1.0);
	matrix.append(1, 1, 2.0);
	matrix.append(2, 1, -1.0);
	matrix.finalize(1);

	matrix.append(1, 2, -1.0);
	matrix.append(2, 2, 1.0);
	matrix.finalize(2);

	return matrix;
}

auto make_identity_matrix(std::size_t size) -> SparseMatrix
{
	SparseMatrix matrix(size, size);
	matrix.reserve(size);

	for (std::size_t index = 0; index < size; ++index) {
		matrix.append(index, index, 1.0);
		matrix.finalize(index);
	}

	return matrix;
}

auto make_zero_matrix(std::size_t size) -> SparseMatrix
{
	SparseMatrix matrix(size, size);
	matrix.reserve(0);
	for (std::size_t index = 0; index < size; ++index) {
		matrix.finalize(index);
	}
	return matrix;
}

auto make_rectangular_matrix() -> SparseMatrix
{
	SparseMatrix matrix(2, 3);
	matrix.reserve(2);

	matrix.append(0, 0, 1.0);
	matrix.finalize(0);

	matrix.append(1, 1, 1.0);
	matrix.finalize(1);

	matrix.finalize(2);
	return matrix;
}

// Block-diagonal adjacency: one unit-weight path per requested block size. Used to drive the
// disconnected/BlockSolver dispatch in lapWrapComponents.
auto make_block_path_adjacency(const std::vector<std::size_t> &block_sizes) -> SparseMatrix
{
	std::size_t n = 0;
	for (std::size_t size : block_sizes)
		n += size;

	std::vector<std::size_t> rows;
	std::vector<std::size_t> cols;
	std::vector<double> values;

	std::size_t offset = 0;
	for (std::size_t size : block_sizes) {
		for (std::size_t k = 0; k + 1 < size; ++k) {
			const std::size_t u = offset + k;
			const std::size_t v = offset + k + 1;
			rows.push_back(u);
			cols.push_back(v);
			values.push_back(1.0);
			rows.push_back(v);
			cols.push_back(u);
			values.push_back(1.0);
		}
		offset += size;
	}

	Vector weights(values.size());
	for (std::size_t k = 0; k < values.size(); ++k)
		weights[k] = values[k];

	return mtrc::sparse(rows, cols, weights, n, n, true);
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

auto assert_residual_close(const SparseMatrix &matrix, const Vector &solution, const Vector &rhs, double tolerance)
	-> void
{
	const Vector residual = matrix * solution - rhs;
	assert(mtrc::numeric::norm(residual) < tolerance);
}

auto exercise_pcg_spd_system() -> void
{
	const auto system = make_spd_system();

	Vector expected(3);
	expected[0] = 1.0;
	expected[1] = 2.0;
	expected[2] = 3.0;

	const Vector rhs = system * expected;
	mtrc::SolverB<double> identity_preconditioner = [](const Vector &input) { return input; };

	std::vector<std::size_t> pcg_iterations(1, 0);
	const Vector solution =
		mtrc::pcg(system, rhs, identity_preconditioner, pcg_iterations, 1e-12F, 50.0);

	mtrc::test::assert_vector_close(solution, expected, 1e-8);
	assert_residual_close(system, solution, rhs, 1e-8);
	assert(pcg_iterations[0] > 0);
	// Tight Krylov bound: CG on an SPD system of dimension n converges in at most n steps.
	assert(pcg_iterations[0] <= 3);

	pcg_iterations[0] = 0;
	const Vector matrix_preconditioned_solution =
		mtrc::pcg(system, rhs, make_identity_matrix(3), pcg_iterations, 1e-12F, 50.0, HUGE_VAL, false, 2);
	mtrc::test::assert_vector_close(matrix_preconditioned_solution, expected, 1e-8);
	assert(pcg_iterations[0] <= 3);

	// Exact preconditioner: passing the system matrix itself makes M^{-1}A = I, so PCG converges
	// in a single iteration. This pins the iteration count to a hand-derivable value.
	pcg_iterations[0] = 0;
	const Vector exactly_preconditioned =
		mtrc::pcg(system, rhs, system, pcg_iterations, 1e-12F, 50.0);
	mtrc::test::assert_vector_close(exactly_preconditioned, expected, 1e-8);
	assert(pcg_iterations[0] == 1);

	// 2x2 known system: [[2,1],[1,2]] x = (1,2) has the exact solution (0,1).
	const auto system2 = make_2x2_spd_system();
	Vector rhs2(2);
	rhs2[0] = 1.0;
	rhs2[1] = 2.0;
	Vector expected2(2);
	expected2[0] = 0.0;
	expected2[1] = 1.0;
	std::vector<std::size_t> iterations2(1, 0);
	const Vector solution2 = mtrc::pcg(system2, rhs2, identity_preconditioner, iterations2, 1e-12F, 50.0);
	mtrc::test::assert_vector_close(solution2, expected2, 1e-8);
	assert(iterations2[0] > 0 && iterations2[0] <= 2);
}

// The solve must be scale invariant: rescaling the right-hand side rescales the solution exactly.
// This guards against the absolute zero-RHS threshold silently flattening small-magnitude systems.
auto exercise_pcg_scale_invariance() -> void
{
	const auto system = make_spd_system();
	Vector expected(3);
	expected[0] = 1.0;
	expected[1] = 2.0;
	expected[2] = 3.0;

	mtrc::SolverB<double> identity_preconditioner = [](const Vector &input) { return input; };

	const double scales[] = {1.0, 1e-4, 1e-8};
	for (double scale : scales) {
		Vector rhs = system * expected;
		rhs *= scale;

		std::vector<std::size_t> iterations(1, 0);
		const Vector solution = mtrc::pcg(system, rhs, identity_preconditioner, iterations, 1e-12F, 50.0);

		Vector target(3);
		for (std::size_t index = 0; index < 3; ++index)
			target[index] = scale * expected[index];

		mtrc::test::assert_vector_close(solution, target, 1e-7 * scale + 1e-15);
	}
}

auto exercise_pcg_guards() -> void
{
	const auto system = make_spd_system();

	Vector rhs(3);
	rhs[0] = 1.0;
	rhs[1] = 2.0;
	rhs[2] = 3.0;

	std::vector<std::size_t> pcg_iterations(1, 0);
	mtrc::SolverB<double> identity_preconditioner = [](const Vector &input) { return input; };

	assert_invalid_argument([&] {
		Vector short_rhs(2);
		(void)mtrc::pcg(system, short_rhs, identity_preconditioner, pcg_iterations);
	});

	assert_invalid_argument([&] {
		(void)mtrc::pcg(make_rectangular_matrix(), Vector(2, 1.0), identity_preconditioner, pcg_iterations);
	});

	assert_invalid_argument([&] {
		mtrc::SolverB<double> bad_preconditioner = [](const Vector &) { return Vector(2, 0.0); };
		(void)mtrc::pcg(system, rhs, bad_preconditioner, pcg_iterations);
	});

	assert_invalid_argument([&] {
		(void)mtrc::pcg(system, rhs, make_identity_matrix(2), pcg_iterations);
	});

	assert_invalid_argument([&] {
		(void)mtrc::pcg(make_zero_matrix(3), rhs, identity_preconditioner, pcg_iterations);
	});

	// A non-finite right-hand side is rejected up front rather than silently iterating on NaN.
	assert_invalid_argument([&] {
		Vector nan_rhs(3, 0.0);
		nan_rhs[1] = std::numeric_limits<double>::quiet_NaN();
		(void)mtrc::pcg(system, nan_rhs, identity_preconditioner, pcg_iterations);
	});

	// A preconditioner that injects a NaN must be caught by the finiteness guards (NaN slips
	// through every relational `< EPS` comparison), not left to poison the iteration silently.
	assert_invalid_argument([&] {
		mtrc::SolverB<double> nan_preconditioner = [](const Vector &input) {
			Vector output = input;
			if (output.size())
				output[0] = std::numeric_limits<double>::quiet_NaN();
			return output;
		};
		(void)mtrc::pcg(system, rhs, nan_preconditioner, pcg_iterations);
	});

	// An exactly-zero right-hand side returns the zero vector without throwing.
	{
		std::vector<std::size_t> iterations(1, 0);
		const Vector solution = mtrc::pcg(system, Vector(3, 0.0), identity_preconditioner, iterations);
		mtrc::test::assert_vector_close(solution, Vector(3, 0.0), 1e-12);
	}

	// maxits == 0 performs no iterations and returns the zero starting guess.
	{
		std::vector<std::size_t> iterations(1, 0);
		const Vector solution = mtrc::pcg(system, rhs, identity_preconditioner, iterations, 1e-12F, 0.0);
		mtrc::test::assert_vector_close(solution, Vector(3, 0.0), 1e-12);
		assert(iterations[0] == 0);
	}
}

auto exercise_laplacian_path_graph() -> void
{
	const auto adjacency = mtrc::sparse(mtrc::path_graph_ijv<double>(4));
	const auto laplacian = mtrc::lap(adjacency);

	assert(laplacian.rows() == 4);
	assert(laplacian.columns() == 4);

	assert(laplacian(0, 0) == 1.0);
	assert(laplacian(1, 1) == 2.0);
	assert(laplacian(2, 2) == 2.0);
	assert(laplacian(3, 3) == 1.0);

	assert(laplacian(0, 1) == -1.0);
	assert(laplacian(1, 0) == -1.0);
	assert(laplacian(1, 2) == -1.0);
	assert(laplacian(2, 1) == -1.0);
	assert(laplacian(2, 3) == -1.0);
	assert(laplacian(3, 2) == -1.0);

	Vector ones(4, 1.0);
	const Vector row_sums = laplacian * ones;
	mtrc::test::assert_vector_close(row_sums, Vector(4, 0.0), 1e-12);
}

// lap() must reject inputs for which the combinatorial Laplacian is undefined.
auto exercise_lap_preconditions() -> void
{
	// Non-square adjacency.
	assert_invalid_argument([&] { (void)mtrc::lap(make_rectangular_matrix()); });

	// Structurally asymmetric adjacency: edge stored only as (1,0)=3, with (0,1)=0.
	SparseMatrix asymmetric(2, 2);
	asymmetric.reserve(1);
	asymmetric.append(1, 0, 3.0);
	asymmetric.finalize(0);
	asymmetric.finalize(1);
	assert_invalid_argument([&] { (void)mtrc::lap(asymmetric); });
}

auto exercise_approxchol_path_graph() -> void
{
	const auto adjacency = mtrc::sparse(mtrc::path_graph_ijv<double>(4));
	const auto laplacian = mtrc::lap(adjacency);

	Vector rhs(4);
	rhs[0] = -1.0;
	rhs[1] = 0.0;
	rhs[2] = 0.0;
	rhs[3] = 1.0;

	// Hand-derived mean-zero potentials of the P4 resistor chain under loads at the endpoints.
	Vector expected_potentials(4);
	expected_potentials[0] = -1.5;
	expected_potentials[1] = -0.5;
	expected_potentials[2] = 0.5;
	expected_potentials[3] = 1.5;

	Vector first_solution;
	for (int trial = 0; trial < 3; ++trial) {
		std::vector<std::size_t> pcg_iterations(1, 0);
		auto solver = mtrc::approxchol_lap(adjacency, pcg_iterations, 1e-10F, 1000.0);
		const Vector solution = solver(rhs, pcg_iterations);

		assert_residual_close(laplacian, solution, rhs, 1e-6);
		assert(std::abs(mtrc::numeric::sum(solution)) < 1e-8);
		assert(pcg_iterations[0] <= 1000);
		mtrc::test::assert_vector_close(solution, expected_potentials, 1e-6);

		// The factorization is randomized, but the solve is deterministic up to PCG tolerance:
		// every rebuild must agree on the same unique mean-zero answer.
		if (trial == 0)
			first_solution = solution;
		else
			mtrc::test::assert_vector_close(solution, first_solution, 1e-7);
	}

	// A second, non-antisymmetric load with independently hand-derived potentials.
	Vector rhs2(4);
	rhs2[0] = 1.0;
	rhs2[1] = -1.0;
	rhs2[2] = -1.0;
	rhs2[3] = 1.0;
	Vector expected2(4);
	expected2[0] = 0.5;
	expected2[1] = -0.5;
	expected2[2] = -0.5;
	expected2[3] = 0.5;
	std::vector<std::size_t> iterations2(1, 0);
	auto solver2 = mtrc::approxchol_lap(adjacency, iterations2, 1e-10F, 1000.0);
	const Vector solution2 = solver2(rhs2, iterations2);
	assert_residual_close(laplacian, solution2, rhs2, 1e-6);
	mtrc::test::assert_vector_close(solution2, expected2, 1e-6);
}

// grid2 is the 2D grid Laplacian fixture called out by the track. A 5x5 grid is connected, so it
// drives the single-component approxChol path on a larger, well-conditioned system.
auto exercise_approxchol_grid2() -> void
{
	const auto adjacency = mtrc::grid2<double>(5, 5);
	const auto laplacian = mtrc::lap(adjacency);
	const std::size_t n = adjacency.rows();
	assert(n == 25);
	assert(mtrc::isConnected(adjacency));

	Vector rhs(n, 0.0);
	rhs[0] = -1.0;
	rhs[n - 1] = 1.0;

	std::vector<std::size_t> pcg_iterations(1, 0);
	auto solver = mtrc::approxchol_lap(adjacency, pcg_iterations, 1e-8F, 1000.0);
	const Vector solution = solver(rhs, pcg_iterations);

	assert_residual_close(laplacian, solution, rhs, 1e-5);
	assert(std::abs(mtrc::numeric::sum(solution)) < 1e-7);
	assert(pcg_iterations[0] > 0 && pcg_iterations[0] <= 1000);
}

// Single isolated node (n == 1): the LLmatp constructor must not dereference an empty column.
auto exercise_approxchol_single_node() -> void
{
	SparseMatrix adjacency(1, 1);
	adjacency.reserve(0);
	adjacency.finalize(0);

	std::vector<std::size_t> pcg_iterations(1, 0);
	auto solver = mtrc::approxchol_lap(adjacency, pcg_iterations);

	Vector rhs(1);
	rhs[0] = 5.0;
	const Vector solution = solver(rhs, pcg_iterations);

	assert(solution.size() == 1);
	assert(std::abs(solution[0]) < 1e-12);
}

// A vertex attached only by a zero-weight (non-)edge violates the strictly-positive-weight
// precondition and must be rejected instead of running into the empty-column out-of-bounds access.
auto exercise_approxchol_degenerate_zero_edge() -> void
{
	SparseMatrix adjacency(3, 3);
	adjacency.reserve(4);

	adjacency.append(1, 0, 1.0);
	adjacency.finalize(0);

	adjacency.append(0, 1, 1.0);
	adjacency.append(2, 1, 0.0);
	adjacency.finalize(1);

	adjacency.append(1, 2, 0.0);
	adjacency.finalize(2);

	std::vector<std::size_t> pcg_iterations(1, 0);
	assert_invalid_argument([&] { (void)mtrc::approxchol_lap(adjacency, pcg_iterations); });
}

// Two small disjoint components exercise the multi-component BlockSolver dispatch and the
// per-component dense-Cholesky (< 50 nodes) branch.
auto exercise_approxchol_disconnected_small() -> void
{
	const auto adjacency = make_block_path_adjacency({3, 3});
	assert(!mtrc::isConnected(adjacency));
	const auto components = mtrc::components(adjacency);
	assert(*std::max_element(components.begin(), components.end()) == 2);

	const auto laplacian = mtrc::lap(adjacency);

	Vector rhs(6, 0.0);
	rhs[0] = -1.0;
	rhs[2] = 1.0;
	rhs[3] = -1.0;
	rhs[5] = 1.0;

	std::vector<std::size_t> pcg_iterations(1, 0);
	auto solver = mtrc::approxchol_lap(adjacency, pcg_iterations, 1e-10F, 1000.0);
	const Vector solution = solver(rhs, pcg_iterations);

	assert_residual_close(laplacian, solution, rhs, 1e-5);
	// Each component carries its own constant nullspace, fixed independently to mean zero.
	assert(std::abs(solution[0] + solution[1] + solution[2]) < 1e-7);
	assert(std::abs(solution[3] + solution[4] + solution[5]) < 1e-7);
}

// Two disjoint components each >= 50 nodes drive the large-component branch of lapWrapComponents,
// which must factorize the per-component submatrix (not the full graph).
auto exercise_approxchol_disconnected_large() -> void
{
	const auto adjacency = make_block_path_adjacency({60, 60});
	const auto components = mtrc::components(adjacency);
	assert(*std::max_element(components.begin(), components.end()) == 2);

	const auto laplacian = mtrc::lap(adjacency);

	Vector rhs(120, 0.0);
	rhs[0] = -1.0;
	rhs[59] = 1.0;
	rhs[60] = -1.0;
	rhs[119] = 1.0;

	std::vector<std::size_t> pcg_iterations(1, 0);
	auto solver = mtrc::approxchol_lap(adjacency, pcg_iterations, 1e-8F, 1000.0);
	const Vector solution = solver(rhs, pcg_iterations);

	assert_residual_close(laplacian, solution, rhs, 1e-4);

	double mean_block_a = 0.0;
	double mean_block_b = 0.0;
	for (std::size_t index = 0; index < 60; ++index) {
		mean_block_a += solution[index];
		mean_block_b += solution[index + 60];
	}
	assert(std::abs(mean_block_a) < 1e-5);
	assert(std::abs(mean_block_b) < 1e-5);
}

// sddmWrapLap solves a symmetric diagonally dominant system by augmenting it to a Laplacian.
// The returned vector must have the full size n (regression for the n-1 truncation).
auto exercise_sddm_wrap_lap() -> void
{
	const auto sddm = make_sddm_system();

	std::vector<std::size_t> pcg_iterations(1, 0);
	auto solver =
		mtrc::sddmWrapLap(mtrc::SolverA<double>(mtrc::approxchol_lap1<double>), sddm, pcg_iterations, 1e-10F, 1000.0);

	Vector rhs(3);
	rhs[0] = 1.0;
	rhs[1] = 2.0;
	rhs[2] = 3.0;

	const Vector solution = solver(rhs, pcg_iterations);

	assert(solution.size() == 3);
	assert_residual_close(sddm, solution, rhs, 1e-6);
}

// Degenerate IJV / conversion inputs must fail fast or return empty, not underflow size_t, drop
// data, or read past an empty range. Regression coverage for the helper-layer guards.
auto exercise_helper_degenerate_guards() -> void
{
	// sparse() with n == 0 returns an empty matrix instead of finalizing column SIZE_MAX.
	{
		const std::vector<std::size_t> empty_i;
		const std::vector<std::size_t> empty_j;
		const Vector empty_v(0);
		const auto empty = mtrc::sparse(empty_i, empty_j, empty_v, std::size_t(0), std::size_t(0), false);
		assert(empty.rows() == 0);
		assert(empty.columns() == 0);
	}

	// path_graph_ijv(0) and (1) have no edges; nnz must be 0 (no 2*(n-1) size_t wraparound), while
	// a real path keeps its 2*(n-1) edge count.
	{
		assert(mtrc::path_graph_ijv<double>(0).nnz == 0);
		assert(mtrc::path_graph_ijv<double>(1).nnz == 0);
		assert(mtrc::path_graph_ijv<double>(4).nnz == 2 * (4 - 1));
	}

	// components() requires a square adjacency (it indexes by row while walking columns).
	assert_invalid_argument([&] { (void)mtrc::components(make_rectangular_matrix()); });

	// The 5-arg IJV constructor rejects an nnz that disagrees with the i/j/v lengths.
	assert_invalid_argument([&] {
		const std::vector<std::size_t> i{0, 1};
		const std::vector<std::size_t> j{0, 1};
		const Vector v(2, 1.0);
		(void)mtrc::IJV<double>(std::size_t(2), std::size_t(3), i, j, v);
	});

	// A negative adjacency edge weight is rejected on every approxchol_lap dispatch arm (here a
	// connected 2-node graph drives the single-component path).
	assert_invalid_argument([&] {
		SparseMatrix negative(2, 2);
		negative.reserve(2);
		negative.append(1, 0, -1.0);
		negative.finalize(0);
		negative.append(0, 1, -1.0);
		negative.finalize(1);
		std::vector<std::size_t> pcg_iterations(1, 0);
		(void)mtrc::approxchol_lap(negative, pcg_iterations);
	});
}

// findmax over columns must not abort on a valid non-increasing column, and must reject a bad axis.
auto exercise_findmax_columnwise() -> void
{
	SparseMatrix matrix(2, 2);
	matrix.reserve(4);
	matrix.append(0, 0, 1.0);
	matrix.append(1, 0, 4.0);
	matrix.finalize(0);
	matrix.append(0, 1, 3.0);
	matrix.append(1, 1, 2.0);
	matrix.finalize(1);

	const auto rowwise = mtrc::findmax(matrix, 1);
	assert(mtrc::test::close_to(rowwise.first, 4.0));
	assert(rowwise.second == 1);

	const auto columnwise = mtrc::findmax(matrix, 2);
	assert(mtrc::test::close_to(columnwise.first, 4.0));
	assert(columnwise.second == 0);

	assert_invalid_argument([&] { (void)mtrc::findmax(matrix, 3); });
}

// grid2 / sum fixture-builder contracts: the bad-argument branches must throw a
// std::exception (they used to throw a bare const char*, escaping every standard
// catch and reaching std::terminate), and grid2's anisotropy weight must actually
// be applied (it used to be silently dropped by calling the 2-arg grid2_ijv).
auto exercise_grid2_and_sum_contract() -> void
{
	// grid2(n < 2) and sum(A, bad-wise) throw std::invalid_argument.
	assert_invalid_argument([&] { (void)mtrc::grid2<double>(1); });
	assert_invalid_argument([&] { (void)mtrc::sum(make_spd_system(), 3); });

	// The isotropy weight scales the n-axis path edges. Before the fix
	// grid2(2, 2, 3.0) was byte-identical to grid2(2, 2, 1.0); the total edge
	// weight must now be strictly larger when isotropy > 1.
	const double total_iso1 = mtrc::numeric::sum(mtrc::sum(mtrc::grid2<double>(2, 2, 1.0), 2));
	const double total_iso3 = mtrc::numeric::sum(mtrc::sum(mtrc::grid2<double>(2, 2, 3.0), 2));
	assert(total_iso3 > total_iso1 + 1e-9);
}

} // namespace

auto main() -> int
{
	exercise_pcg_spd_system();
	exercise_pcg_scale_invariance();
	exercise_pcg_guards();
	exercise_laplacian_path_graph();
	exercise_lap_preconditions();
	exercise_approxchol_path_graph();
	exercise_approxchol_grid2();
	exercise_approxchol_single_node();
	exercise_approxchol_degenerate_zero_edge();
	exercise_approxchol_disconnected_small();
	exercise_approxchol_disconnected_large();
	exercise_sddm_wrap_lap();
	exercise_helper_degenerate_guards();
	exercise_findmax_columnwise();
	exercise_grid2_and_sum_contract();

	return 0;
}
