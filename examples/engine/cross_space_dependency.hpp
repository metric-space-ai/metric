// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Cross-Space Dependency hero application — shared implementation.
//
// Thesis
// ------
// METRIC can decide whether two *different* finite metric spaces are dependent
// without ever forcing both record types into a single common embedding. Here
// the two spaces are deliberately heterogeneous:
//
//   * LEFT  — operations event logs, modelled as variable-length token strings
//             compared with edit (Levenshtein) distance, `mtrc::Edit<char>`.
//   * RIGHT — process curves, modelled as real-valued sensor traces compared
//             with Time-Warp Edit Distance, `mtrc::TWED<double>`.
//
// There is no natural shared vector space for "a log string" and "a sensor
// curve"; each lives in its own metric space with its own native distance. The
// dependence question — "do the two modalities co-vary across the paired
// observations?" — is answered by Multiscale Graph Correlation (MGC), exposed
// through the engine intent `mtrc::compare(left_space, right_space)`.
//
// Statistical contract
// --------------------
// MGC is a *dependence test*, NOT a metric and NOT a distance. The engine
// returns a single sample MGC statistic in [-1, 1] (~1 strong dependence, ~0
// none) and, by design, NO p-value (see
// docs/examples/correlation-between-spaces.md). This application adds the
// missing significance layer the way a practitioner should: a seeded
// permutation test built entirely on top of the public `compare()` API. We do
// NOT reach into MGC internals and we never treat MGC as a distance.
//
//   statistic  = MGC sample statistic on the true observation pairing (effect)
//   null pairing = the SAME left space compared against the right space whose
//                records have been randomly re-paired (observation labels
//                shuffled). Re-pairing is the textbook independence null.
//   p_value    = (1 + #{ permuted statistic >= observed }) / (1 + permutations)
//                — the standard add-one estimator: one-sided, monotone in the
//                evidence, never exactly zero, and valid for any permutation
//                count.
//
// Everything is deterministic: MGC's `compare()` path is the exact computation
// (not the sampling `estimate`), and every random draw is seeded, so the whole
// report is reproducible bit-for-bit.

#ifndef METRIC_EXAMPLES_ENGINE_CROSS_SPACE_DEPENDENCY_HPP
#define METRIC_EXAMPLES_ENGINE_CROSS_SPACE_DEPENDENCY_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include <metric/engine.hpp>
#include <metric/metric/catalog.hpp>

namespace cross_space {

// ---------------------------------------------------------------------------
// Record types and native metrics.
// ---------------------------------------------------------------------------

using EventLog = std::string;            // left  record: operations event log
using ProcessCurve = std::vector<double>; // right record: process / sensor curve

using LogMetric = mtrc::Edit<char>;       // edit distance over event tokens
using CurveMetric = mtrc::TWED<double>;   // time-warp edit distance over curves

// TWED stiffness/penalty kept fixed for the whole application so the curve
// metric space is well defined and reproducible.
inline auto curve_metric() -> CurveMetric { return CurveMetric(/*penalty=*/0.5, /*elastic=*/1.0); }

// A paired, heterogeneous dataset. `logs[i]` and `curves[i]` are the two views
// of observation `i`; `latent[i]` is the hidden regime that (optionally) drives
// both views and is retained only for diagnostics — the dependence test never
// sees it.
struct Dataset {
	std::vector<EventLog> logs;
	std::vector<ProcessCurve> curves;
	std::vector<double> latent;

	auto size() const -> std::size_t { return logs.size(); }
};

enum class Coupling {
	coupled,   // logs and curves share one latent regime  -> dependent
	decoupled, // logs and curves use independent latents   -> independent
};

// ---------------------------------------------------------------------------
// Domain generators.
//
// The generators are tuned so that the dependence lives entirely in STRUCTURE,
// never in the cheap scalar summaries a naive baseline would reach for:
//
//   * Event logs have near-constant length (small jitter that is independent of
//     the regime), so "log length" carries no regime information. The regime
//     instead controls WHICH event tokens appear and WHERE, which edit distance
//     sees but a length/character-sum reduction does not.
//   * Process curves are mean-preserving: a regime-driven early overshoot is
//     compensated by a later dip, so the curve MEAN is ~constant across regimes
//     while the SHAPE (timing of the excursion) encodes the regime. TWED sees
//     the shape; a "curve mean/area" reduction does not.
//
// Net effect: native metric spaces track the latent strongly, naive scalar
// vectorisation is blind to it.
// ---------------------------------------------------------------------------

namespace detail {

// Deterministic per-stream RNG so independent quantities never share a stream.
// std::mt19937_64 is a standardised engine: its raw output sequence is identical
// on every conforming toolchain for a given seed. The std::*_distribution and
// std::shuffle adaptors are NOT portable across standard libraries, so the
// helpers below derive every draw directly from the raw 64-bit output, making
// the generated datasets, jitters, and permutations reproducible bit-for-bit on
// every platform (the only remaining cross-platform variable is the engine's own
// floating-point determinism, which the core gate already relies on).
inline auto stream(std::uint64_t seed, std::uint64_t salt) -> std::mt19937_64
{
	return std::mt19937_64(seed ^ (salt * 0x9E3779B97F4A7C15ULL + 0x632BE59BD9B4E019ULL));
}

// Portable uniform double in [0, 1) using the top 53 bits (one full mantissa).
inline auto next_unit(std::mt19937_64 &rng) -> double
{
	return static_cast<double>(rng() >> 11) * (1.0 / 9007199254740992.0); // 2^53
}

// Portable uniform integer in [0, bound). `bound` is small here so modulo bias
// is negligible; determinism across toolchains is what matters.
inline auto next_below(std::mt19937_64 &rng, std::uint64_t bound) -> std::uint64_t
{
	return rng() % bound;
}

// Portable Fisher-Yates shuffle (replaces non-portable std::shuffle).
template <typename T> inline auto portable_shuffle(std::vector<T> &values, std::mt19937_64 &rng) -> void
{
	for (std::size_t index = values.size(); index > 1; --index) {
		const auto pick = static_cast<std::size_t>(next_below(rng, static_cast<std::uint64_t>(index)));
		std::swap(values[index - 1], values[pick]);
	}
}

constexpr std::size_t kLogBaseLength = 28; // event tokens per log before jitter
constexpr std::size_t kCurveLength = 32;   // samples per process curve

} // namespace detail

// Build one event log for a given latent severity in [0, 1].
//
// Layout: 'S' (start) ... body ... 'E' (end). The body is normal-operation
// tokens ('h' heat, 'H' hold, 'c' cool) into which anomaly tokens ('j' jitter,
// 'o' overpressure, 'x' fault) are woven. The number and identity of anomaly
// tokens rise with severity; their POSITIONS shift with severity too, so two
// logs from nearby regimes align well under edit distance and far-apart regimes
// do not. `jitter` adds 0..2 trailing 'H' holds independent of severity, giving
// length a small regime-free wobble.
inline auto make_event_log(double severity, int jitter) -> EventLog
{
	static const char normal_cycle[3] = {'h', 'H', 'c'};
	static const char anomaly_codes[3] = {'j', 'o', 'x'};

	const std::size_t body = detail::kLogBaseLength;
	const auto anomaly_count = static_cast<std::size_t>(std::lround(severity * static_cast<double>(body) * 0.6));

	EventLog log;
	log.reserve(body + 4);
	log.push_back('S');

	// Anomalies concentrate in a window whose centre slides with severity, so
	// the edit alignment between two logs depends on |severity_a - severity_b|.
	const auto window_centre = static_cast<double>(body) * (0.25 + 0.5 * severity);
	for (std::size_t position = 0; position < body; ++position) {
		const double distance_to_centre = std::abs(static_cast<double>(position) - window_centre);
		const bool in_window = distance_to_centre < (static_cast<double>(anomaly_count) / 2.0 + 0.5);
		if (in_window && anomaly_count > 0) {
			// Pick the anomaly code by how deep we are into the window so the
			// token identity (not just count) carries the regime.
			const auto code_index = static_cast<std::size_t>(distance_to_centre) % 3;
			log.push_back(anomaly_codes[code_index]);
		} else {
			log.push_back(normal_cycle[position % 3]);
		}
	}

	for (int extra = 0; extra < jitter; ++extra) {
		log.push_back('H');
	}
	log.push_back('E');
	return log;
}

// Build one process curve for a given latent severity in [0, 1].
//
// Base ramp + settle. The regime adds a mean-preserving excursion: a positive
// overshoot bump early, an equal-area negative dip later. Amplitude grows with
// severity and the excursion's phase shifts with severity, so TWED (timing
// aware) tracks the regime while the curve mean stays ~constant.
inline auto make_process_curve(double severity) -> ProcessCurve
{
	const std::size_t length = detail::kCurveLength;
	ProcessCurve curve(length, 0.0);

	const double amplitude = 3.0 * severity;
	const double phase = static_cast<double>(length) * (0.2 + 0.4 * severity);

	for (std::size_t index = 0; index < length; ++index) {
		const auto t = static_cast<double>(index);
		// Smooth base ramp that saturates — same for every record.
		const double base = 2.0 * (1.0 - std::exp(-t / 6.0));
		// Mean-preserving excursion: + bump near `phase`, - dip near `phase+gap`.
		const double bump = amplitude * std::exp(-std::pow(t - phase, 2.0) / 8.0);
		const double dip = amplitude * std::exp(-std::pow(t - (phase + 6.0), 2.0) / 8.0);
		curve[index] = base + bump - dip;
	}
	return curve;
}

// Generate a paired dataset of `count` observations.
//
// `coupling == coupled`   : both views derive from one shared latent regime.
// `coupling == decoupled` : curves derive from an independent latent stream, so
//                           the two spaces are statistically independent.
inline auto generate_dataset(std::size_t count, Coupling coupling, std::uint64_t seed) -> Dataset
{
	auto latent_rng = detail::stream(seed, 1);
	auto jitter_rng = detail::stream(seed, 2);
	auto decoupled_rng = detail::stream(seed, 3);

	Dataset data;
	data.logs.reserve(count);
	data.curves.reserve(count);
	data.latent.reserve(count);

	for (std::size_t index = 0; index < count; ++index) {
		const double log_latent = detail::next_unit(latent_rng);
		const double curve_latent =
			(coupling == Coupling::coupled) ? log_latent : detail::next_unit(decoupled_rng);
		const int jitter = static_cast<int>(detail::next_below(jitter_rng, 3));

		data.logs.push_back(make_event_log(log_latent, jitter));
		data.curves.push_back(make_process_curve(curve_latent));
		data.latent.push_back(log_latent);
	}
	return data;
}

// Return a copy of `data` whose curve order is shuffled (observation labels on
// the right space permuted), destroying the pairing while preserving each space
// in isolation. A single draw from the independence null distribution.
inline auto permute_curves(const Dataset &data, std::uint64_t seed) -> Dataset
{
	Dataset shuffled = data;
	std::vector<std::size_t> order(data.size());
	std::iota(order.begin(), order.end(), std::size_t{0});

	auto rng = detail::stream(seed, 99);
	detail::portable_shuffle(order, rng);

	for (std::size_t index = 0; index < data.size(); ++index) {
		shuffled.curves[index] = data.curves[order[index]];
	}
	return shuffled;
}

// ---------------------------------------------------------------------------
// METRIC path: MGC dependence statistic + permutation-test significance.
// ---------------------------------------------------------------------------

struct DependenceReport {
	double statistic{};        // observed MGC sample statistic in [-1, 1]
	double null_mean{};        // mean MGC over the permutation null
	double null_sd{};          // std of MGC over the permutation null
	double standardized{};     // (statistic - null_mean) / null_sd  (effect z)
	double p_value{};          // permutation p-value (add-one estimator)
	std::size_t permutations{}; // number of permutations drawn
	std::size_t record_count{}; // paired observations
};

// The observed MGC statistic computed via the public `compare()` intent — the
// headline API a user calls. The permutation null below reuses precomputed
// distance matrices for speed and is verified to return this exact value.
inline auto observed_statistic(const Dataset &data) -> double
{
	auto log_space = mtrc::make_space(data.logs, LogMetric{});
	auto curve_space = mtrc::make_space(data.curves, curve_metric());
	return mtrc::compare(log_space, curve_space, mtrc::stats::correlate::mgc_options{}).value;
}

// The two native pairwise-distance matrices (logs under edit distance, curves
// under TWED), materialised once through the public DistanceTable +
// provider_symmetric_distance_matrix helpers. These are the legitimate inputs
// to the MGC dependence test; nothing here treats MGC as a distance.
struct DistanceMatrices {
	mtrc::DistanceMatrix<double> left;  // event logs / edit distance
	mtrc::DistanceMatrix<double> right; // process curves / TWED
};

inline auto build_distance_matrices(const Dataset &data) -> DistanceMatrices
{
	auto log_space = mtrc::make_space(data.logs, LogMetric{});
	auto curve_space = mtrc::make_space(data.curves, curve_metric());
	mtrc::space::storage::DistanceTable<decltype(log_space)> log_table(log_space);
	mtrc::space::storage::DistanceTable<decltype(curve_space)> curve_table(curve_space);
	return DistanceMatrices{
		mtrc::space::storage::provider_symmetric_distance_matrix<double>(log_table),
		mtrc::space::storage::provider_symmetric_distance_matrix<double>(curve_table)};
}

// MGC dependence statistic between two precomputed distance matrices. This is
// the same computation the engine performs inside `compare()` (verified by the
// equivalence assertion in the example); reusing it lets the permutation null
// avoid recomputing distances on every draw.
inline auto mgc_statistic(const mtrc::DistanceMatrix<double> &left, const mtrc::DistanceMatrix<double> &right) -> double
{
	return mtrc::MGC_direct{}(left, right);
}

// Permutation test for cross-space dependence. The left (log) distance matrix
// is fixed; each draw re-pairs the right (curve) observations by permuting the
// rows/columns of its distance matrix — the textbook independence null — and
// recomputes the MGC statistic.
inline auto permutation_test(const Dataset &data, std::size_t permutations, std::uint64_t seed) -> DependenceReport
{
	DependenceReport report;
	report.record_count = data.size();
	report.permutations = permutations;

	const auto matrices = build_distance_matrices(data);
	report.statistic = mgc_statistic(matrices.left, matrices.right);

	std::vector<std::size_t> order(data.size());
	std::iota(order.begin(), order.end(), std::size_t{0});
	auto rng = detail::stream(seed, 7);

	std::vector<double> null_samples;
	null_samples.reserve(permutations);
	std::size_t at_least_as_extreme = 0;

	mtrc::DistanceMatrix<double> permuted(data.size());
	for (std::size_t draw = 0; draw < permutations; ++draw) {
		detail::portable_shuffle(order, rng);
		for (std::size_t row = 0; row < data.size(); ++row) {
			permuted(row, row) = 0.0;
			for (std::size_t column = row + 1; column < data.size(); ++column) {
				permuted(row, column) = matrices.right(order[row], order[column]);
			}
		}
		const double null_statistic = mgc_statistic(matrices.left, permuted);
		null_samples.push_back(null_statistic);
		if (null_statistic >= report.statistic) {
			++at_least_as_extreme;
		}
	}

	const double count = static_cast<double>(permutations);
	if (permutations > 0) {
		const double sum = std::accumulate(null_samples.begin(), null_samples.end(), 0.0);
		report.null_mean = sum / count;
		double variance = 0.0;
		for (const double sample : null_samples) {
			const double centered = sample - report.null_mean;
			variance += centered * centered;
		}
		variance /= count;
		report.null_sd = std::sqrt(variance);
		report.standardized =
			report.null_sd > 0.0 ? (report.statistic - report.null_mean) / report.null_sd : 0.0;
	}
	report.p_value =
		(1.0 + static_cast<double>(at_least_as_extreme)) / (1.0 + count);
	return report;
}

// ---------------------------------------------------------------------------
// Baselines: what dependence detection looks like WITHOUT METRIC's native
// metric spaces — i.e. forcing both modalities into naive numeric vectors.
// ---------------------------------------------------------------------------

inline auto log_length(const EventLog &log) -> double { return static_cast<double>(log.size()); }

inline auto curve_mean(const ProcessCurve &curve) -> double
{
	if (curve.empty()) {
		return 0.0;
	}
	return std::accumulate(curve.begin(), curve.end(), 0.0) / static_cast<double>(curve.size());
}

inline auto pearson(const std::vector<double> &left, const std::vector<double> &right) -> double
{
	const auto n = static_cast<double>(left.size());
	if (left.size() < 2 || left.size() != right.size()) {
		return 0.0;
	}
	const double left_mean = std::accumulate(left.begin(), left.end(), 0.0) / n;
	const double right_mean = std::accumulate(right.begin(), right.end(), 0.0) / n;
	double covariance = 0.0;
	double left_var = 0.0;
	double right_var = 0.0;
	for (std::size_t index = 0; index < left.size(); ++index) {
		const double left_centered = left[index] - left_mean;
		const double right_centered = right[index] - right_mean;
		covariance += left_centered * right_centered;
		left_var += left_centered * left_centered;
		right_var += right_centered * right_centered;
	}
	if (left_var <= 0.0 || right_var <= 0.0) {
		return 0.0;
	}
	return covariance / std::sqrt(left_var * right_var);
}

struct BaselineReport {
	double pearson_r{};        // naive scalar Pearson correlation
	double pearson_p_value{};  // its permutation p-value (|r|, two-sided)
	double vectorized_mgc{};   // MGC on forced common Euclidean vectors
	std::size_t permutations{};
};

// Encode a log as a fixed-length numeric vector by zero-padded character codes
// — the naive "just turn the string into numbers" move.
inline auto encode_log_vector(const EventLog &log, std::size_t width) -> std::vector<double>
{
	std::vector<double> encoded(width, 0.0);
	for (std::size_t index = 0; index < log.size() && index < width; ++index) {
		encoded[index] = static_cast<double>(static_cast<unsigned char>(log[index]));
	}
	return encoded;
}

// Encode a curve as a fixed-length numeric vector by zero-padded samples.
inline auto encode_curve_vector(const ProcessCurve &curve, std::size_t width) -> std::vector<double>
{
	std::vector<double> encoded(width, 0.0);
	for (std::size_t index = 0; index < curve.size() && index < width; ++index) {
		encoded[index] = curve[index];
	}
	return encoded;
}

inline auto baseline_report(const Dataset &data, std::size_t permutations, std::uint64_t seed) -> BaselineReport
{
	BaselineReport report;
	report.permutations = permutations;

	std::vector<double> log_scalar;
	std::vector<double> curve_scalar;
	log_scalar.reserve(data.size());
	curve_scalar.reserve(data.size());
	for (std::size_t index = 0; index < data.size(); ++index) {
		log_scalar.push_back(log_length(data.logs[index]));
		curve_scalar.push_back(curve_mean(data.curves[index]));
	}

	report.pearson_r = pearson(log_scalar, curve_scalar);

	// Permutation p-value for the naive scalar correlation (two-sided via |r|).
	std::vector<double> shuffled = curve_scalar;
	auto rng = detail::stream(seed, 11);
	std::size_t at_least_as_extreme = 0;
	const double observed_abs = std::abs(report.pearson_r);
	for (std::size_t draw = 0; draw < permutations; ++draw) {
		detail::portable_shuffle(shuffled, rng);
		if (std::abs(pearson(log_scalar, shuffled)) >= observed_abs) {
			++at_least_as_extreme;
		}
	}
	report.pearson_p_value =
		(1.0 + static_cast<double>(at_least_as_extreme)) / (1.0 + static_cast<double>(permutations));

	// Forced-common-vector MGC: same dependence test, but on naively vectorised
	// records (zero-padded character codes vs zero-padded samples) under plain
	// Euclidean distance instead of the native metric spaces.
	std::size_t log_width = 0;
	std::size_t curve_width = 0;
	for (const auto &log : data.logs) {
		log_width = std::max(log_width, log.size());
	}
	for (const auto &curve : data.curves) {
		curve_width = std::max(curve_width, curve.size());
	}
	std::vector<std::vector<double>> log_vectors;
	std::vector<std::vector<double>> curve_vectors;
	log_vectors.reserve(data.size());
	curve_vectors.reserve(data.size());
	for (std::size_t index = 0; index < data.size(); ++index) {
		log_vectors.push_back(encode_log_vector(data.logs[index], log_width));
		curve_vectors.push_back(encode_curve_vector(data.curves[index], curve_width));
	}
	auto log_vector_space = mtrc::make_space(log_vectors, mtrc::Euclidean<double>{});
	auto curve_vector_space = mtrc::make_space(curve_vectors, mtrc::Euclidean<double>{});
	report.vectorized_mgc =
		mtrc::compare(log_vector_space, curve_vector_space, mtrc::stats::correlate::mgc_options{}).value;

	return report;
}

} // namespace cross_space

#endif // METRIC_EXAMPLES_ENGINE_CROSS_SPACE_DEPENDENCY_HPP
