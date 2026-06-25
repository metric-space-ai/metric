// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// =============================================================================
//  modify::dynamics  --  Finite Metric Dynamics
// -----------------------------------------------------------------------------
//  Randomness, diffusion and reverse diffusion expressed as an evolution
//  *over the structure of a finite metric space*. Redif measure dynamics is the
//  coordinate-free noise surface; this file also contains vector-signal
//  perturbation helpers for Euclidean/vector-valued records.
//
//  The single, load-bearing idea of this operator is:
//
//      Probability is a DERIVED INTERPRETATION of the geometry, not a primitive.
//
//  A finite metric space gives us only pairwise distances d(i, j). From those
//  distances -- and nothing else -- we derive a heat kernel
//
//      W_ij = exp(-(d_ij / eps)^2)         (symmetric affinity, k-NN sparsified)
//
//  and row-normalise it into a Markov transition matrix
//
//      P_ij = W_ij / sum_j W_ij            (row-stochastic: sum_j P_ij = 1)
//
//  P_ij is the probability of stepping from node i to node j. It exists only
//  because a metric exists. Every stochastic object in this file -- the random
//  walk, the forward degradation transition, the stationary distribution -- is built
//  on top of P, i.e. on top of the geometry. We never start from a probability
//  axiom; we read probability off the metric.
//
//  Three workflows are provided, all over the *fixed* metric structure (graph):
//
//    * metric_transition / metric_random_walk
//          geometry -> probability. Builds P and its stationary distribution,
//          and walks the graph by sampling neighbours ~ P_i.
//
//    * metric_diffuse        (forward / vector-signal perturbation)
//          a signal carried by the nodes is smoothed along the graph (heat flow)
//          while seeded coordinate perturbation is applied. This is a vector
//          record specialization, not the universal notion of metric noise.
//
//    * metric_reconstruct    (reverse / reconstruction)
//          the same graph heat flow run without coordinate perturbation
	//          contracts injected high-frequency fluctuations back onto the metric structure.
	//          Coordinate-free inverse disorder over arbitrary records belongs to
	//          Redif measure dynamics in redif.hpp.
//
//  The operator is header-only and depends only on the engine core and the C++
//  standard library: no LAPACK, no numeric back-end, no new dependencies. The
//  pseudo-random generator and its normal/uniform draws are implemented inline
//  so that a (seed, schedule) pair reproduces a run bit-for-bit on any platform.
// =============================================================================

#ifndef _METRIC_MODIFY_DYNAMICS_FINITE_DYNAMICS_HPP
#define _METRIC_MODIFY_DYNAMICS_FINITE_DYNAMICS_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>

namespace mtrc::modify::dynamics {

// ----------------------------------------------------------------------------
// Configuration
// ----------------------------------------------------------------------------

/// Schedule controlling how dynamics evolve over a finite metric space.
///
/// Every field is geometry- or reproducibility-facing; none of them assumes a
/// probability distribution as input. `bandwidth == 0` asks the operator to
/// derive the heat-kernel scale `eps` from the data (mean k-NN distance), which
/// keeps the transition probabilities a pure function of the metric.
inline constexpr std::size_t default_metric_transition_max_dense_records = 4096;

struct dynamics_schedule {
	std::size_t neighbors = 6;   ///< k for the metric neighbourhood graph.
	std::size_t steps = 12;      ///< number of dynamics steps to integrate.
	double diffusivity = 0.5;    ///< alpha in (0, 1]; per-step graph mixing weight.
	double perturbation_scale = 0.0; ///< std-dev of the vector-coordinate perturbation term.
	std::uint64_t seed = 0;      ///< seed for every stochastic draw (full reproducibility).
	double bandwidth = 0.0;      ///< heat-kernel eps; 0 => derive from mean k-NN distance.
	std::size_t max_dense_records = default_metric_transition_max_dense_records; ///< 0 => no dense preflight limit.
};

// ----------------------------------------------------------------------------
// Deterministic, platform-independent pseudo-randomness.
//
// std::normal_distribution / std::uniform_*_distribution are NOT specified to
// produce identical sequences across standard-library implementations, so a
// portable "same seed => same trajectory" guarantee requires our own draws.
// splitmix64 + Box-Muller is tiny, well understood and fully reproducible.
// ----------------------------------------------------------------------------
namespace detail {

class deterministic_rng {
  public:
	explicit deterministic_rng(std::uint64_t seed) : state_(seed) {}

	auto next_u64() -> std::uint64_t
	{
		state_ += 0x9E3779B97F4A7C15ULL;
		std::uint64_t z = state_;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}

	/// Uniform double in [0, 1) with 53 bits of resolution.
	auto uniform() -> double { return static_cast<double>(next_u64() >> 11) * (1.0 / 9007199254740992.0); }

	/// Standard normal via Box-Muller (caches the second variate).
	auto standard_normal() -> double
	{
		if (has_cached_) {
			has_cached_ = false;
			return cached_;
		}
		// Guard the log against u1 == 0.
		double u1 = uniform();
		if (u1 < 1e-300) {
			u1 = 1e-300;
		}
		const double u2 = uniform();
		const double radius = std::sqrt(-2.0 * std::log(u1));
		const double angle = 2.0 * 3.14159265358979323846 * u2;
		cached_ = radius * std::sin(angle);
		has_cached_ = true;
		return radius * std::cos(angle);
	}
  private:
	std::uint64_t state_;
	double cached_ = 0.0;
	bool has_cached_ = false;
};

/// Detects records that behave like a finite real vector (size + indexing +
/// arithmetic element type), which is what `metric_diffuse`/`metric_reconstruct` evolve.
template <typename Record, typename = void> struct is_vector_record : std::false_type {};

template <typename Record>
struct is_vector_record<
	Record, std::void_t<typename Record::value_type, decltype(std::declval<const Record &>().size()),
						 decltype(std::declval<const Record &>()[std::size_t{0}])>>
	: std::is_arithmetic<typename Record::value_type> {};

template <typename Record> constexpr bool is_vector_record_v = is_vector_record<Record>::value;

} // namespace detail

// ----------------------------------------------------------------------------
// Geometry -> probability: the metric transition operator.
// ----------------------------------------------------------------------------

/// The Markov transition operator derived purely from a finite metric space.
///
/// `probabilities` is row-stochastic (each row sums to 1) and is the only
/// stochastic primitive in the whole operator. `stationary` is its invariant
/// distribution pi (pi P = pi); for a symmetric affinity it is proportional to
/// node degree. `affinity` and `degree` are exposed so callers can compute the
/// graph Dirichlet energy used as a diffusion invariant.
struct MetricTransition {
	std::size_t node_count = 0;
	std::size_t neighbors = 0;
	double bandwidth = 0.0;
	std::vector<std::vector<double>> affinity;      ///< symmetric W (zero diagonal).
	std::vector<std::vector<double>> probabilities; ///< row-stochastic P = D^{-1} W.
	std::vector<double> degree;                     ///< d_i = sum_j W_ij.
	std::vector<double> stationary;                 ///< invariant distribution pi.

	auto size() const -> std::size_t { return node_count; }

	/// Sum of row `i` of P. Exposed so invariants ("== 1") are checkable.
	auto row_sum(std::size_t i) const -> double
	{
		double total = 0.0;
		for (const auto value : probabilities[i]) {
			total += value;
		}
		return total;
	}
};

namespace detail {

/// Pairwise distance matrix from the metric -- the *only* thing we read from the
/// space. Works for any MetricSpaceLike record/metric pair.
template <typename Space> auto distance_matrix(const Space &space) -> std::vector<std::vector<double>>
{
	const std::size_t n = space.size();
	std::vector<std::vector<double>> distances(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		const auto id_i = space.id(i);
		for (std::size_t j = i + 1; j < n; ++j) {
			const auto id_j = space.id(j);
			const double d = static_cast<double>(space.distance(id_i, id_j));
			distances[i][j] = d;
			distances[j][i] = d;
		}
	}
	return distances;
}

} // namespace detail

/// Build the geometry-derived transition operator P from a finite metric space.
///
/// Steps, all functions of the distances alone:
///   1. heat-kernel scale eps (given, or mean of the k smallest non-zero
///      distances per node),
///   2. k-NN sparsified affinity W_ij = exp(-(d_ij/eps)^2), symmetrised by max,
///   3. row-normalise to P, and read the invariant distribution from the degree.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metric_transition(const Space &space, const dynamics_schedule &schedule) -> MetricTransition
{
	const std::size_t n = space.size();
	if (n == 0) {
		throw std::invalid_argument("metric_transition requires a non-empty metric space");
	}
	if (schedule.max_dense_records > 0 && n > schedule.max_dense_records) {
		throw MetricInputError("metric_transition dense construction exceeds max_dense_records: records=" +
							   std::to_string(n) + " max_dense_records=" +
							   std::to_string(schedule.max_dense_records));
	}
	const std::size_t k = std::min(schedule.neighbors, n > 0 ? n - 1 : 0);

	const auto distances = detail::distance_matrix(space);

	// --- 1. heat-kernel scale eps, derived from the metric when not provided ---
	double bandwidth = schedule.bandwidth;
	if (!(bandwidth > 0.0)) {
		double accum = 0.0;
		std::size_t count = 0;
		for (std::size_t i = 0; i < n; ++i) {
			std::vector<double> row = distances[i];
			std::sort(row.begin(), row.end());
			// row[0] is the zero self-distance; take the next k positive entries.
			for (std::size_t t = 1; t <= k && t < row.size(); ++t) {
				accum += row[t];
				++count;
			}
		}
		bandwidth = (count > 0 && accum > 0.0) ? accum / static_cast<double>(count) : 1.0;
		if (!(bandwidth > 0.0)) {
			bandwidth = 1.0;
		}
	}

	// --- 2. k-NN sparsified, symmetric heat-kernel affinity W ---
	std::vector<std::vector<double>> affinity(n, std::vector<double>(n, 0.0));
	const double inv_eps2 = 1.0 / (bandwidth * bandwidth);
	for (std::size_t i = 0; i < n; ++i) {
		// indices of the k nearest neighbours of i (excluding itself)
		std::vector<std::size_t> order;
		order.reserve(n);
		for (std::size_t j = 0; j < n; ++j) {
			if (j != i) {
				order.push_back(j);
			}
		}
		std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
			if (distances[i][a] != distances[i][b]) {
				return distances[i][a] < distances[i][b];
			}
			return a < b; // deterministic tie-break
		});
		const std::size_t limit = std::min(k, order.size());
		for (std::size_t t = 0; t < limit; ++t) {
			const std::size_t j = order[t];
			const double d = distances[i][j];
			const double w = std::exp(-d * d * inv_eps2);
			// symmetrise by max so the local graph is undirected.
			if (w > affinity[i][j]) {
				affinity[i][j] = w;
				affinity[j][i] = w;
			}
		}
	}

	// --- 3. row-normalise to the transition matrix P and read off pi ---
	MetricTransition transition;
	transition.node_count = n;
	transition.neighbors = k;
	transition.bandwidth = bandwidth;
	transition.affinity = affinity;
	transition.degree.assign(n, 0.0);
	transition.probabilities.assign(n, std::vector<double>(n, 0.0));

	double degree_total = 0.0;
	for (std::size_t i = 0; i < n; ++i) {
		double row = 0.0;
		for (std::size_t j = 0; j < n; ++j) {
			row += affinity[i][j];
		}
		transition.degree[i] = row;
		degree_total += row;
		if (row > 0.0) {
			for (std::size_t j = 0; j < n; ++j) {
				transition.probabilities[i][j] = affinity[i][j] / row;
			}
		} else {
			// isolated node: stay put (still row-stochastic).
			transition.probabilities[i][i] = 1.0;
		}
	}

	// Invariant distribution. For a symmetric W with P = D^{-1} W the stationary
	// distribution is pi_i ∝ degree_i; fall back to uniform if degenerate.
	transition.stationary.assign(n, 0.0);
	if (degree_total > 0.0) {
		for (std::size_t i = 0; i < n; ++i) {
			transition.stationary[i] = transition.degree[i] / degree_total;
		}
	} else {
		for (std::size_t i = 0; i < n; ++i) {
			transition.stationary[i] = 1.0 / static_cast<double>(n);
		}
	}

	return transition;
}

// ----------------------------------------------------------------------------
// Random walk over the metric space: the purest geometry -> probability demo.
// ----------------------------------------------------------------------------

/// Result of walking the metric graph by repeatedly sampling neighbours ~ P_i.
struct RandomWalkResult {
	std::size_t start = 0;
	std::size_t steps = 0;
	std::size_t walkers = 0;
	std::uint64_t seed = 0;
	std::vector<std::size_t> visit_counts;   ///< total visits per node.
	std::vector<double> empirical;           ///< visit_counts normalised to a distribution.
	double total_variation_to_stationary = 0.0; ///< 1/2 sum_i |empirical_i - pi_i|.
};

/// Walk the metric graph: each step jumps to a neighbour chosen with probability
/// P_i, accumulating how often each node is visited. With enough steps the
/// empirical visit distribution converges to the geometry-derived stationary
/// distribution -- "probability is the long-run geometry of the walk".
inline auto metric_random_walk(const MetricTransition &transition, std::size_t start, std::size_t steps,
							   std::uint64_t seed, std::size_t walkers = 1) -> RandomWalkResult
{
	const std::size_t n = transition.node_count;
	if (n == 0) {
		throw std::invalid_argument("metric_random_walk requires a non-empty transition operator");
	}
	if (start >= n) {
		throw std::invalid_argument("metric_random_walk start index out of range");
	}
	if (walkers == 0) {
		walkers = 1;
	}

	RandomWalkResult result;
	result.start = start;
	result.steps = steps;
	result.walkers = walkers;
	result.seed = seed;
	result.visit_counts.assign(n, 0);

	detail::deterministic_rng rng(seed);
	std::size_t total_visits = 0;
	for (std::size_t w = 0; w < walkers; ++w) {
		std::size_t current = start;
		for (std::size_t t = 0; t < steps; ++t) {
			// sample next node from the geometry-derived categorical P_current
			const double u = rng.uniform();
			double cumulative = 0.0;
			std::size_t next = current;
			const auto &row = transition.probabilities[current];
			for (std::size_t j = 0; j < n; ++j) {
				cumulative += row[j];
				if (u < cumulative) {
					next = j;
					break;
				}
			}
			current = next;
			result.visit_counts[current] += 1;
			++total_visits;
		}
	}

	result.empirical.assign(n, 0.0);
	if (total_visits > 0) {
		for (std::size_t i = 0; i < n; ++i) {
			result.empirical[i] = static_cast<double>(result.visit_counts[i]) / static_cast<double>(total_visits);
		}
	}
	double tv = 0.0;
	for (std::size_t i = 0; i < n; ++i) {
		tv += std::abs(result.empirical[i] - transition.stationary[i]);
	}
	result.total_variation_to_stationary = 0.5 * tv;
	return result;
}

// ----------------------------------------------------------------------------
// Dynamics of a signal carried by the nodes (forward / reverse).
// ----------------------------------------------------------------------------

/// A recorded dynamics run over the fixed metric structure.
///
/// `frames` is the full trajectory of the node signal (frames.size() == steps+1,
/// each frame is node_count x dimension). `dirichlet_energy[t]` is the graph
/// Dirichlet energy of frame t, the diffusion invariant: it is monotonically
/// non-increasing under pure heat flow (reconstruction) and grows under noisy
/// degradation. `result` is the engine MappingResult for the terminal space,
/// carrying one-to-one lineage back to the source records.
template <typename Space> struct DynamicsTrajectory {
	using record_type = typename Space::record_type;
	using metric_type = typename Space::metric_type;
	using space_type = MetricSpace<record_type, metric_type>;

	std::string operator_name;                          ///< "metric_diffusion_forward" / "...reverse".
	std::string direction;                              ///< "forward" / "reverse".
	std::size_t steps = 0;
	std::uint64_t seed = 0;
	std::size_t dimension = 0;
	std::vector<std::vector<std::vector<double>>> frames; ///< (steps+1) x n x dim.
	std::vector<double> dirichlet_energy;               ///< per-frame graph Dirichlet energy.
	MappingResult<space_type> result;                   ///< terminal space + lineage.

	auto size() const -> std::size_t { return result.size(); }
};

namespace detail {

/// Extract the node signal X (n x dim) from vector-valued records.
template <typename Space> auto signal_from_space(const Space &space) -> std::vector<std::vector<double>>
{
	const std::size_t n = space.size();
	std::vector<std::vector<double>> signal(n);
	std::size_t dim = 0;
	for (std::size_t i = 0; i < n; ++i) {
		const auto &record = space.record(space.id(i));
		const std::size_t d = static_cast<std::size_t>(record.size());
		if (i == 0) {
			dim = d;
		} else if (d != dim) {
			throw std::invalid_argument("modify::dynamics requires equal-length vector records");
		}
		signal[i].resize(dim);
		for (std::size_t c = 0; c < dim; ++c) {
			signal[i][c] = static_cast<double>(record[c]);
		}
	}
	return signal;
}

/// Graph Dirichlet energy E(x) = sum_{i<j} W_ij ||x_i - x_j||^2. A signal that is
/// smooth along the metric structure has low energy; coordinate perturbation raises it.
inline auto dirichlet_energy(const MetricTransition &transition, const std::vector<std::vector<double>> &signal)
	-> double
{
	const std::size_t n = transition.node_count;
	double energy = 0.0;
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = i + 1; j < n; ++j) {
			const double w = transition.affinity[i][j];
			if (w == 0.0) {
				continue;
			}
			double sq = 0.0;
			for (std::size_t c = 0; c < signal[i].size(); ++c) {
				const double delta = signal[i][c] - signal[j][c];
				sq += delta * delta;
			}
			energy += w * sq;
		}
	}
	return energy;
}

/// One deterministic heat-flow (smoothing) step: x_i <- (1-a) x_i + a sum_j P_ij x_j.
/// This is the metric Laplacian smoothing S = (1-a) I + a P -- the contraction
/// shared by forward drift and reverse reconstruction.
inline auto heat_step(const MetricTransition &transition, const std::vector<std::vector<double>> &signal,
					  double alpha) -> std::vector<std::vector<double>>
{
	const std::size_t n = transition.node_count;
	const std::size_t dim = signal.empty() ? 0 : signal[0].size();
	std::vector<std::vector<double>> next(n, std::vector<double>(dim, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		const auto &row = transition.probabilities[i];
		for (std::size_t c = 0; c < dim; ++c) {
			double neighbour = 0.0;
			for (std::size_t j = 0; j < n; ++j) {
				if (row[j] != 0.0) {
					neighbour += row[j] * signal[j][c];
				}
			}
			next[i][c] = (1.0 - alpha) * signal[i][c] + alpha * neighbour;
		}
	}
	return next;
}

/// Build a terminal MetricSpace whose records carry the evolved signal, with the
/// original record shape/type preserved (we copy each source record and overwrite
/// its components), plus one-to-one lineage back to the source ids.
template <typename Space>
auto terminal_mapping(const Space &space, const std::vector<std::vector<double>> &signal, std::string mapping,
					  std::string strategy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using record_type = typename Space::record_type;
	using metric_type = typename Space::metric_type;
	using target_space_type = MetricSpace<record_type, metric_type>;

	const std::size_t n = space.size();
	std::vector<record_type> records;
	records.reserve(n);
	for (std::size_t i = 0; i < n; ++i) {
		record_type record = space.record(space.id(i)); // preserve type & shape
		for (std::size_t c = 0; c < signal[i].size(); ++c) {
			record[c] = static_cast<typename record_type::value_type>(signal[i][c]);
		}
		records.push_back(std::move(record));
	}

	auto lineage = ::mtrc::one_to_one_lineage(space);
	target_space_type derived(std::move(records), space.metric());
	return core::make_mapping_result(std::move(derived), std::move(lineage.source_records),
									 std::move(lineage.representative_records), space.size(), false, std::move(mapping),
									 std::move(strategy), "metric_space",
									 core::metric_traits<typename Space::metric_type>::law, false,
									 "finite-dynamics terminal space; records are evolved in-sample from the source "
									 "space and retain the source metric; not invertible");
}

} // namespace detail

/// Forward dynamics: degradation of a node signal over the metric structure.
///
/// Each step combines deterministic heat flow (drift along the graph) with
/// seeded vector-coordinate perturbation scaled by `perturbation_scale`. The metric
/// structure stays fixed; the signal carried by the nodes loses structure step by
/// step. This overload is a vector-record specialization of finite-space
/// dynamics; coordinate-free noise treatment lives in Redif measure dynamics.
/// The terminal MappingResult is the degraded finite metric space.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metric_diffuse(const Space &space, const dynamics_schedule &schedule, const MetricTransition &transition)
	-> DynamicsTrajectory<Space>
{
	static_assert(detail::is_vector_record_v<typename Space::record_type>,
				  "modify::dynamics::metric_diffuse evolves a vector signal and needs vector records");

	// The node signal is sized by the space, but the heat/Dirichlet steps iterate
	// transition.node_count. A transition built from a differently-sized space
	// would read/write the signal out of bounds, so require an exact match.
	if (transition.node_count != space.size()) {
		throw std::invalid_argument(
			"modify::dynamics::metric_diffuse: transition node count must match the space record count");
	}

	auto signal = detail::signal_from_space(space);
	const std::size_t dim = signal.empty() ? 0 : signal[0].size();

	std::vector<std::vector<std::vector<double>>> frames;
	std::vector<double> energy;
	frames.reserve(schedule.steps + 1);
	energy.reserve(schedule.steps + 1);

	frames.push_back(signal);
	energy.push_back(detail::dirichlet_energy(transition, signal));

	detail::deterministic_rng rng(schedule.seed);
	for (std::size_t t = 0; t < schedule.steps; ++t) {
		signal = detail::heat_step(transition, signal, schedule.diffusivity);
		if (schedule.perturbation_scale > 0.0) {
			for (auto &node : signal) {
				for (auto &component : node) {
					component += schedule.perturbation_scale * rng.standard_normal();
				}
			}
		}
		frames.push_back(signal);
		energy.push_back(detail::dirichlet_energy(transition, signal));
	}

	auto result =
		detail::terminal_mapping(space, signal, "metric_diffusion_forward", "graph_heat_flow+coordinate_perturbation");
	return DynamicsTrajectory<Space>{"metric_diffusion_forward", "forward",     schedule.steps, schedule.seed,
									 dim,                        std::move(frames), std::move(energy),
									 std::move(result)};
}

/// Convenience overload: derive the transition operator from the space first.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metric_diffuse(const Space &space, const dynamics_schedule &schedule) -> DynamicsTrajectory<Space>
{
	return metric_diffuse(space, schedule, metric_transition(space, schedule));
}

/// Reverse dynamics: reconstruction of a degraded signal over the metric
/// structure. Pure graph heat flow (no noise) is a low-pass filter on the graph:
/// it contracts the high-frequency fluctuations injected by `metric_diffuse`
/// back onto the metric structure. This is the dependency-free sibling of the
/// Redif measure dynamics for coordinate-free inverse disorder.
///
/// `degraded` is the starting (noisy) space; `transition` should be the operator
/// of the structure being reconstructed onto (typically the original space).
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metric_reconstruct(const Space &degraded, const dynamics_schedule &schedule, const MetricTransition &transition)
	-> DynamicsTrajectory<Space>
{
	static_assert(detail::is_vector_record_v<typename Space::record_type>,
				  "modify::dynamics::metric_reconstruct evolves a vector signal and needs vector records");

	// The node signal is sized by `degraded`, but the heat/Dirichlet steps iterate
	// transition.node_count. A transition built from a differently-sized space
	// would read/write the signal out of bounds, so require an exact match.
	if (transition.node_count != degraded.size()) {
		throw std::invalid_argument(
			"modify::dynamics::metric_reconstruct: transition node count must match the space record count");
	}

	auto signal = detail::signal_from_space(degraded);
	const std::size_t dim = signal.empty() ? 0 : signal[0].size();

	std::vector<std::vector<std::vector<double>>> frames;
	std::vector<double> energy;
	frames.reserve(schedule.steps + 1);
	energy.reserve(schedule.steps + 1);

	frames.push_back(signal);
	energy.push_back(detail::dirichlet_energy(transition, signal));

	for (std::size_t t = 0; t < schedule.steps; ++t) {
		signal = detail::heat_step(transition, signal, schedule.diffusivity);
		frames.push_back(signal);
		energy.push_back(detail::dirichlet_energy(transition, signal));
	}

	auto result = detail::terminal_mapping(degraded, signal, "metric_diffusion_reverse", "graph_heat_flow");
	return DynamicsTrajectory<Space>{"metric_diffusion_reverse", "reverse",         schedule.steps, schedule.seed,
									 dim,                        std::move(frames), std::move(energy),
									 std::move(result)};
}

/// Convenience overload: derive the transition operator from `degraded`.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto metric_reconstruct(const Space &degraded, const dynamics_schedule &schedule) -> DynamicsTrajectory<Space>
{
	return metric_reconstruct(degraded, schedule, metric_transition(degraded, schedule));
}

} // namespace mtrc::modify::dynamics

namespace mtrc {
using DynamicsSchedule = modify::dynamics::dynamics_schedule;
using MetricTransition = modify::dynamics::MetricTransition;
using RandomWalkResult = modify::dynamics::RandomWalkResult;
template <typename Space> using DynamicsTrajectory = modify::dynamics::DynamicsTrajectory<Space>;
using modify::dynamics::metric_diffuse;
using modify::dynamics::metric_random_walk;
using modify::dynamics::metric_reconstruct;
using modify::dynamics::metric_transition;
} // namespace mtrc

#endif
