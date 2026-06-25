// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// =============================================================================
//  Finite Metric Dynamics  --  hero example for mtrc::modify::dynamics
// -----------------------------------------------------------------------------
//  Randomness, disorder, forward diffusion and reverse diffusion, all expressed
//  as an evolution over the structure of a finite metric space. The thesis the
//  example proves, line by assertable line:
//
//        Probability is a derived interpretation of the geometry,
//        not a primitive.
//
//  Act 1  Geometry -> probability
//         Derive a Markov transition operator P from pairwise distances alone,
//         show it is row-stochastic, that its stationary distribution is the
//         long-run geometry of a random walk over the metric graph, and that it
//         is invariant under any distance-preserving change of the records.
//
//  Act 2  Forward vector-signal dynamics / degradation
//         Evolve a signal carried by the nodes with graph heat flow plus seeded
//         coordinate perturbation. Structure decays; the run is reproducible.
//
//  Act 3  Reverse diffusion / reconstruction
//         Run the same graph heat flow without coordinate perturbation. As a
//         low-pass filter on the metric graph it contracts fluctuations back
//         onto the structure. Reconstruction beats the degraded input.
//
//  Every claim below is checked with `require`, which fails the process (exit 1)
//  independently of NDEBUG, so this example doubles as a regression test.
// =============================================================================

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <metric/engine.hpp>
#include <metric/metric/catalog/vector/Standards.hpp>

#include "finite_metric_dynamics_fixture.hpp"

namespace {

int g_checks = 0;

// NDEBUG-independent assertion: this example is also wired in as a ctest target.
void require(bool condition, const std::string &message)
{
	++g_checks;
	if (!condition) {
		std::cerr << "FAIL: " << message << "\n";
		std::exit(1);
	}
}

using Signal = std::vector<std::vector<double>>;

// Read the node signal (the record vectors) out of a finite metric space.
template <typename Space> auto signal_of(const Space &space) -> Signal
{
	Signal signal;
	signal.reserve(space.size());
	for (std::size_t i = 0; i < space.size(); ++i) {
		const auto &record = space.record(space.id(i));
		signal.emplace_back(record.begin(), record.end());
	}
	return signal;
}

// Mean squared error between two equally shaped node signals.
auto mse(const Signal &lhs, const Signal &rhs) -> double
{
	double accum = 0.0;
	std::size_t terms = 0;
	for (std::size_t i = 0; i < lhs.size(); ++i) {
		for (std::size_t c = 0; c < lhs[i].size(); ++c) {
			const double delta = lhs[i][c] - rhs[i][c];
			accum += delta * delta;
			++terms;
		}
	}
	return terms == 0 ? 0.0 : accum / static_cast<double>(terms);
}

auto signals_equal(const Signal &lhs, const Signal &rhs) -> bool
{
	if (lhs.size() != rhs.size()) {
		return false;
	}
	for (std::size_t i = 0; i < lhs.size(); ++i) {
		if (lhs[i].size() != rhs[i].size()) {
			return false;
		}
		for (std::size_t c = 0; c < lhs[i].size(); ++c) {
			if (lhs[i][c] != rhs[i][c]) { // bit-for-bit: determinism is exact
				return false;
			}
		}
	}
	return true;
}

// Discrete Shannon entropy (nats) of a probability vector -- a scalar read-out
// of how spread the geometry-derived distribution is.
auto entropy(const std::vector<double> &distribution) -> double
{
	double h = 0.0;
	for (const double p : distribution) {
		if (p > 0.0) {
			h -= p * std::log(p);
		}
	}
	return h;
}

} // namespace

int main()
{
	std::cout.setf(std::ios::fixed);
	std::cout.precision(4);

	// -------------------------------------------------------------------------
	// Build the finite metric space from the deterministic fixture.
	// -------------------------------------------------------------------------
	const auto records = finite_metric_dynamics_fixture::make_records();
	const auto clean = records; // ground-truth signal for the reconstruction test
	auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});

	const std::size_t n = space.size();
	require(n == finite_metric_dynamics_fixture::node_count, "fixture node count");

	mtrc::DynamicsSchedule structure;
	structure.neighbors = 4;  // k for the metric neighbourhood graph
	structure.bandwidth = 0.0; // derive the heat-kernel scale from the metric

	std::cout << "== Finite Metric Dynamics ==\n";
	std::cout << "nodes = " << n << ", dimension = " << finite_metric_dynamics_fixture::dimension
			  << ", neighbors k = " << structure.neighbors << "\n\n";

	// =========================================================================
	// Act 1 -- Geometry -> probability.
	// =========================================================================
	const auto transition = mtrc::metric_transition(space, structure);

	require(transition.node_count == n, "transition node count");
	require(transition.bandwidth > 0.0, "heat-kernel bandwidth derived from metric is positive");

	// (1a) P is row-stochastic and non-negative: a probability for every step.
	for (std::size_t i = 0; i < n; ++i) {
		require(std::abs(transition.row_sum(i) - 1.0) < 1e-9, "row of P sums to 1 (probability from geometry)");
		for (std::size_t j = 0; j < n; ++j) {
			require(transition.probabilities[i][j] >= 0.0, "transition probabilities are non-negative");
		}
	}

	// (1b) A constant signal is a fixed point of P (rows sum to 1). Diffusion
	// conserves the equilibrium -- the harmonic invariant.
	{
		std::vector<double> constant(n, 3.5);
		for (std::size_t i = 0; i < n; ++i) {
			double mixed = 0.0;
			for (std::size_t j = 0; j < n; ++j) {
				mixed += transition.probabilities[i][j] * constant[j];
			}
			require(std::abs(mixed - 3.5) < 1e-9, "constant signal is a fixed point of diffusion");
		}
	}

	// (1c) The stationary distribution is invariant under P: pi P = pi.
	require(std::abs([&] { double s = 0.0; for (double p : transition.stationary) s += p; return s; }() - 1.0) < 1e-9,
			"stationary distribution sums to 1");
	{
		std::vector<double> pushed(n, 0.0);
		for (std::size_t j = 0; j < n; ++j) {
			for (std::size_t i = 0; i < n; ++i) {
				pushed[j] += transition.stationary[i] * transition.probabilities[i][j];
			}
		}
		double drift = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			drift += std::abs(pushed[i] - transition.stationary[i]);
		}
		require(drift < 1e-9, "stationary distribution is invariant under P (pi P = pi)");
	}

	// (1d) Probability is a function of geometry ONLY: translate every record by
	// a constant. Euclidean distances are unchanged, so P must be identical.
	{
		auto shifted_records = records;
		for (auto &r : shifted_records) {
			r[0] += 10.0;
			r[1] -= 4.0;
		}
		auto shifted_space = mtrc::make_space(shifted_records, mtrc::Euclidean<double>{});
		const auto shifted_transition = mtrc::metric_transition(shifted_space, structure);
		double max_diff = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = 0; j < n; ++j) {
				max_diff = std::max(max_diff,
									std::abs(transition.probabilities[i][j] - shifted_transition.probabilities[i][j]));
			}
		}
		require(max_diff < 1e-12, "P is invariant under distance-preserving record changes (geometry, not coordinates)");
	}

	// (1e) Random walk over the metric graph: empirical visit frequencies
	// converge to the geometry-derived stationary distribution.
	const std::uint64_t walk_seed = 20240617;
	const auto walk = mtrc::metric_random_walk(transition, /*start=*/0, /*steps=*/4000, walk_seed, /*walkers=*/16);
	require(walk.total_variation_to_stationary < 0.06,
			"random walk converges to the stationary distribution derived from geometry");

	// Determinism of the walk: same seed reproduces it exactly; a different seed
	// explores a different sample path.
	const auto walk_again = mtrc::metric_random_walk(transition, 0, 4000, walk_seed, 16);
	require(walk.visit_counts == walk_again.visit_counts, "random walk is reproducible for a fixed seed");
	const auto walk_other = mtrc::metric_random_walk(transition, 0, 4000, walk_seed + 1, 16);
	require(walk.visit_counts != walk_other.visit_counts, "a different seed yields a different sample path");

	std::cout << "Act 1  geometry -> probability\n";
	std::cout << "  heat-kernel bandwidth eps = " << transition.bandwidth << "\n";
	std::cout << "  H(stationary)            = " << entropy(transition.stationary) << " nats (max "
			  << std::log(static_cast<double>(n)) << ")\n";
	std::cout << "  walk TV to stationary    = " << walk.total_variation_to_stationary << "\n\n";

	// =========================================================================
	// Act 2 -- Forward vector-signal dynamics / degradation.
	// =========================================================================
	mtrc::DynamicsSchedule forward = structure;
	forward.steps = 6;
	forward.diffusivity = 0.05; // light drift along the graph
	forward.perturbation_scale = 0.06; // seeded coordinate perturbation for vector records
	forward.seed = 1234;

	const auto forward_run = mtrc::metric_diffuse(space, forward, transition);

	// (2a) Output shape is preserved through the dynamics.
	require(forward_run.frames.size() == forward.steps + 1, "forward trajectory has steps+1 frames");
	for (const auto &frame : forward_run.frames) {
		require(frame.size() == n, "every frame carries all nodes");
		for (const auto &node : frame) {
			require(node.size() == finite_metric_dynamics_fixture::dimension, "every node keeps its dimension");
		}
	}
	require(forward_run.result.size() == n, "degraded space keeps the node count");
	require(forward_run.dimension == finite_metric_dynamics_fixture::dimension, "trajectory records the dimension");

	// (2b) Lineage: one-to-one back to the source records.
	const auto &fwd_result = forward_run.result;
	require(fwd_result.source_record_count == n, "lineage records the source count");
	require(fwd_result.source_records.size() == n, "lineage has one row per derived record");
	require(fwd_result.representative_records.size() == n, "lineage has one representative per derived record");
	require(fwd_result.mapping == "metric_diffusion_forward", "forward mapping name");
	for (std::size_t i = 0; i < n; ++i) {
		require(fwd_result.source_records[i].size() == 1, "one-to-one lineage row");
		require(fwd_result.source_records[i][0] == space.id(i), "lineage maps derived i back to source i");
		require(fwd_result.representative_records[i] == space.id(i), "representative i is source i");
	}

	// (2c) Determinism: same (seed, schedule) reproduces the run bit-for-bit;
	// a different seed produces a different degraded space.
	const auto forward_repeat = mtrc::metric_diffuse(space, forward, transition);
	require(signals_equal(signal_of(forward_run.result.space), signal_of(forward_repeat.result.space)),
			"forward diffusion is reproducible for a fixed seed");
	mtrc::DynamicsSchedule forward_other = forward;
	forward_other.seed = 9999;
	const auto forward_other_run = mtrc::metric_diffuse(space, forward_other, transition);
	require(!signals_equal(signal_of(forward_run.result.space), signal_of(forward_other_run.result.space)),
			"a different seed degrades the space differently");

	// (2d) Degradation actually happened: the signal moved off the manifold and
	// the graph Dirichlet energy rose above the clean baseline.
	const Signal degraded = signal_of(forward_run.result.space);
	const double degraded_mse = mse(degraded, clean);
	require(degraded_mse > 0.0, "forward diffusion degraded the signal");
	require(forward_run.dirichlet_energy.back() > forward_run.dirichlet_energy.front(),
			"coordinate perturbation raised the graph Dirichlet energy");

	std::cout << "Act 2  forward vector-signal dynamics / degradation\n";
	std::cout << "  steps = " << forward.steps << ", coordinate_perturbation = " << forward.perturbation_scale << ", seed = " << forward.seed
			  << "\n";
	std::cout << "  Dirichlet energy " << forward_run.dirichlet_energy.front() << " -> "
			  << forward_run.dirichlet_energy.back() << "\n";
	std::cout << "  MSE(degraded, clean)     = " << degraded_mse << "\n\n";

	// =========================================================================
	// Act 3 -- Reverse diffusion / reconstruction.
	// =========================================================================
	// The metric structure is the fixed prior, so we reconstruct using the
	// transition operator of the original geometry.
	mtrc::DynamicsSchedule reverse = structure;
	reverse.steps = 40;
	reverse.diffusivity = 0.18; // gentle contraction toward the structure
	reverse.perturbation_scale = 0.0; // reverse dynamics applies no coordinate perturbation

	const auto &degraded_space = forward_run.result.space;
	const auto reverse_run = mtrc::metric_reconstruct(degraded_space, reverse, transition);

	// (3a) Shape and lineage of the reconstruction.
	require(reverse_run.frames.size() == reverse.steps + 1, "reverse trajectory has steps+1 frames");
	require(reverse_run.result.size() == n, "reconstructed space keeps the node count");
	require(reverse_run.result.mapping == "metric_diffusion_reverse", "reverse mapping name");
	for (std::size_t i = 0; i < n; ++i) {
		require(reverse_run.result.source_records[i][0] == degraded_space.id(i),
				"reverse lineage maps reconstruction i back to degraded i");
	}

	// (3b) Maximum principle (provable, holds for any graph): each reverse step
	// replaces a node value with a convex combination of node values, so it can
	// never create a new per-coordinate extremum. This is the guaranteed
	// contraction behind reverse diffusion; it follows purely from P being
	// row-stochastic, i.e. purely from the geometry.
	for (std::size_t c = 0; c < finite_metric_dynamics_fixture::dimension; ++c) {
		double lo = reverse_run.frames.front()[0][c];
		double hi = lo;
		for (const auto &node : reverse_run.frames.front()) {
			lo = std::min(lo, node[c]);
			hi = std::max(hi, node[c]);
		}
		for (const auto &frame : reverse_run.frames) {
			for (const auto &node : frame) {
				require(node[c] >= lo - 1e-9 && node[c] <= hi + 1e-9,
						"reverse diffusion obeys the maximum principle (no new extrema)");
			}
		}
	}

	// (3c) On a structured signal the graph Dirichlet energy decreases: the
	// reverse flow is a low-pass filter that removes the injected high-frequency
	// fluctuations. (Observed here; guaranteed in the small-step limit.)
	for (std::size_t t = 1; t < reverse_run.dirichlet_energy.size(); ++t) {
		require(reverse_run.dirichlet_energy[t] <= reverse_run.dirichlet_energy[t - 1] + 1e-9,
				"reverse diffusion does not increase the Dirichlet energy");
	}

	// (3d) Reconstruction beats the degraded input: there is a reverse time at
	// which the signal is markedly closer to the clean manifold.
	double best_mse = degraded_mse;
	std::size_t best_step = 0;
	for (std::size_t t = 0; t < reverse_run.frames.size(); ++t) {
		const double m = mse(reverse_run.frames[t], clean);
		if (m < best_mse) {
			best_mse = m;
			best_step = t;
		}
	}
	require(best_step > 0, "reconstruction improves on the degraded input");
	require(best_mse < 0.85 * degraded_mse, "reverse diffusion recovers structure (>= 15% MSE reduction)");

	// (3e) The structure need not be handed to us: rebuild the graph from the
	// degraded observations alone (the convenience overload, mirroring the
	// adaptive Redif measure dynamics) and confirm reconstruction still improves.
	const auto adaptive_run = mtrc::metric_reconstruct(degraded_space, reverse);
	double adaptive_best = degraded_mse;
	std::size_t adaptive_step = 0;
	for (std::size_t t = 0; t < adaptive_run.frames.size(); ++t) {
		const double m = mse(adaptive_run.frames[t], clean);
		if (m < adaptive_best) {
			adaptive_best = m;
			adaptive_step = t;
		}
	}
	require(adaptive_step > 0 && adaptive_best < degraded_mse,
			"reconstruction from the degraded geometry alone still improves on the degraded input");

	std::cout << "Act 3  reverse diffusion / reconstruction\n";
	std::cout << "  best reverse step        = " << best_step << " / " << reverse.steps << "\n";
	std::cout << "  MSE(reconstructed,clean) = " << best_mse << " (was " << degraded_mse << ")\n";
	std::cout << "  improvement              = " << (100.0 * (1.0 - best_mse / degraded_mse)) << "%\n";
	std::cout << "  adaptive (graph from degraded) best MSE = " << adaptive_best << " at step " << adaptive_step
			  << " (" << (100.0 * (1.0 - adaptive_best / degraded_mse)) << "% better)\n";
	std::cout << "  Dirichlet energy " << reverse_run.dirichlet_energy.front() << " -> "
			  << reverse_run.dirichlet_energy.back() << "\n\n";

	std::cout << "All " << g_checks << " invariants hold. Probability stayed a read-out of the metric.\n";
	return 0;
}
