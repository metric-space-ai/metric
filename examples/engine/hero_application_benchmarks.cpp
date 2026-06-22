// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// =====================================================================================
//  METRIC hero-application benchmark / evidence track
// =====================================================================================
//
//  Goal: produce *defensible* evidence -- not marketing numbers -- that the four
//  METRIC hero applications are CORRECT, REPRODUCIBLE and PERFORMANT. Four hero
//  workloads are exercised against a naive vector/Euclidean baseline:
//
//    1. Mixed Records        -- composed domain metric vs flat numeric Euclidean
//    2. Condition Monitoring -- TWED finite metric space vs Euclidean window vector
//    3. Cross-Space Dependency -- MGC dependence between two metric spaces vs raw pairing
//    4. Metric Mapping       -- PHATE diffusion-geometry targets driven by the metric
//
//  Two kinds of evidence are emitted, kept strictly separate:
//
//    * DETERMINISTIC evidence (stdout, byte-stable, checked into
//      docs/examples/hero-application-benchmarks.md):
//        - correctness: metric-space 1-NN recovers the intended family where the
//          naive vector baseline does not (observed match/mismatch counts + margin);
//        - work reduction: a workflow that consumes the pairwise structure K times
//          re-evaluates the domain metric K*n^2 times naively, but n^2 times when the
//          structure is materialized once (DistanceTable). Both columns are OBSERVED
//          with a shared-counter metric wrapper, never asserted by construction.
//      These are integers / exact ratios, independent of platform and wall-clock.
//
//    * WALL-CLOCK evidence (stderr, machine-dependent, deliberately NOT checked in):
//        - real measured per-evaluation cost of the metric vs the baseline, median of
//          repeated passes. This quantifies the price of correctness without ever
//          entering the byte-stable artifact.
//
//  Modes: smoke (default) keeps every fixture tiny and uses few timing repetitions so
//  the binary finishes well under a second and is safe for CI. Setting the environment
//  variable METRIC_HERO_BENCH_FULL=1 raises the timing repetition counts for a richer
//  wall-clock signal; the deterministic evidence is unchanged (same fixtures).
//
//  The program is fully deterministic (hashed-noise generators, no <random> locale
//  drift) and depends only on the header-only METRIC core. It returns 0 only if every
//  correctness assertion and every work-reduction invariant holds.
// =====================================================================================

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>
#include <metric/benchmarks/counting_metric.hpp>
#include <metric/benchmarks/report.hpp>

namespace {

using mtrc::benchmarks::BenchmarkReport;
using mtrc::benchmarks::BenchmarkRunMetadata;
using mtrc::benchmarks::PerformanceRow;
using mtrc::benchmarks::WorkflowEvidenceRow;

// ------------------------------------------------------------------------------------
// Wall-clock harness (out-of-band; never enters the byte-stable artifact).
// ------------------------------------------------------------------------------------
using clock_type = std::chrono::steady_clock;
volatile double g_sink = 0.0; // defeats dead-code elimination of timed work

auto full_mode() -> bool
{
	const char *value = std::getenv("METRIC_HERO_BENCH_FULL");
	return value != nullptr && value[0] != '\0' && value[0] != '0';
}

// Median wall-clock (nanoseconds) of `reps` calls to `work`, after one warmup call.
// `work` returns a checksum that is funneled into g_sink so the optimizer cannot
// elide the measured computation.
template <typename Work> auto median_ns(int reps, Work &&work) -> double
{
	g_sink += work(); // warmup (also primes caches)
	std::vector<double> samples;
	samples.reserve(static_cast<std::size_t>(reps));
	for (int rep = 0; rep < reps; ++rep) {
		const auto start = clock_type::now();
		const double value = work();
		const auto stop = clock_type::now();
		g_sink += value;
		samples.push_back(std::chrono::duration<double, std::nano>(stop - start).count());
	}
	std::sort(samples.begin(), samples.end());
	return samples[samples.size() / 2];
}

std::vector<std::string> g_timing_lines; // collected, printed to stderr at the end

void record_timing(const std::string &line) { g_timing_lines.push_back(line); }

auto close_to(double lhs, double rhs, double tolerance = 1e-9) -> bool
{
	return std::abs(lhs - rhs) <= tolerance * (1.0 + std::abs(lhs) + std::abs(rhs));
}

// ------------------------------------------------------------------------------------
// Uniform, OBSERVED work-reduction measurement.
//
// A hero workflow consumes the full pairwise distance structure K times (neighbour
// table, relation graph, structural summary, ...). The naive pipeline re-evaluates the
// domain metric for every consumer; a materialized DistanceTable evaluates it once and
// serves every later consumer from the cache. Both eval counts are observed with a
// shared-counter metric wrapper, and the two consumers' checksums must agree (exact
// cache).
// ------------------------------------------------------------------------------------
struct WorkReduction {
	PerformanceRow row;
	mtrc::space::storage::representation_diagnostics diagnostics;
};

template <typename Records, typename Metric>
auto measure_work_reduction(std::string benchmark, std::string workload, const Records &records, Metric base,
							std::size_t shared_consumers, std::string notes) -> WorkReduction
{
	const std::size_t n = records.size();

	// --- Naive: each consumer re-evaluates the metric over all ordered pairs. ---
	auto naive_metric = mtrc::benchmarks::make_counting_metric(base);
	const auto naive_counter = naive_metric.counter();
	auto naive_space = mtrc::make_space(records, naive_metric);
	double naive_checksum = 0.0;
	for (std::size_t pass = 0; pass < shared_consumers; ++pass) {
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = 0; j < n; ++j) {
				naive_checksum += static_cast<double>(naive_space.distance(naive_space.id(i), naive_space.id(j)));
			}
		}
	}
	const std::size_t naive_evals = *naive_counter;

	// --- Materialized: build the pairwise structure once, serve every consumer. ---
	auto cached_metric = mtrc::benchmarks::make_counting_metric(base);
	const auto cached_counter = cached_metric.counter();
	auto cached_space = mtrc::make_space(records, cached_metric);
	mtrc::space::storage::DistanceTable<decltype(cached_space)> table(cached_space);
	const std::size_t build_evals = *cached_counter;
	double cached_checksum = 0.0;
	for (std::size_t pass = 0; pass < shared_consumers; ++pass) {
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = 0; j < n; ++j) {
				cached_checksum += static_cast<double>(table.distance(cached_space.id(i), cached_space.id(j)));
			}
		}
	}
	const std::size_t cache_miss = *cached_counter - build_evals;

	WorkReduction result;
	result.row.benchmark = std::move(benchmark);
	result.row.workload = std::move(workload);
	result.row.record_count = n;
	result.row.shared_operations = shared_consumers;
	result.row.naive_distance_evaluations = naive_evals;
	result.row.materialized_distance_evaluations = build_evals;
	result.row.cache_miss_evaluations = cache_miss;
	result.row.evaluation_reduction_factor =
		build_evals == 0 ? 0.0 : static_cast<double>(naive_evals) / static_cast<double>(build_evals);
	result.row.exact_match = close_to(naive_checksum, cached_checksum);
	result.row.notes = std::move(notes);
	result.diagnostics = table.diagnostics();

	// Invariants that make the row trustworthy: the naive pipeline re-evaluates the
	// metric for every consumer, the materialized cache eliminates re-evaluation
	// entirely (zero cache misses), and the cached and recomputed checksums agree.
	assert(naive_evals == shared_consumers * n * n);
	assert(build_evals >= 1 && build_evals <= n * n);
	assert(cache_miss == 0);
	assert(result.row.exact_match);
	return result;
}

// =====================================================================================
//  Hero 1: Mixed Records
// =====================================================================================
namespace mixed {

struct IndustrialRecord {
	std::array<double, 3> temperature_summary{};
	std::string status;
	std::string message;
	std::array<double, 4> spectrum{};
	std::vector<double> curve;
};

auto euclidean3(const std::array<double, 3> &lhs, const std::array<double, 3> &rhs) -> double
{
	double squared = 0.0;
	for (std::size_t i = 0; i < lhs.size(); ++i) {
		const auto delta = lhs[i] - rhs[i];
		squared += delta * delta;
	}
	return std::sqrt(squared);
}

auto edit_distance(const std::string &lhs, const std::string &rhs) -> std::size_t
{
	std::vector<std::size_t> previous(rhs.size() + 1);
	std::iota(previous.begin(), previous.end(), std::size_t{0});
	for (std::size_t i = 1; i <= lhs.size(); ++i) {
		std::vector<std::size_t> current(rhs.size() + 1);
		current[0] = i;
		for (std::size_t j = 1; j <= rhs.size(); ++j) {
			const auto sub = previous[j - 1] + (lhs[i - 1] == rhs[j - 1] ? 0 : 1);
			current[j] = std::min({sub, previous[j] + 1, current[j - 1] + 1});
		}
		previous = std::move(current);
	}
	return previous.back();
}

auto cumulative_transport(const std::array<double, 4> &lhs, const std::array<double, 4> &rhs) -> double
{
	double cumulative = 0.0;
	double distance = 0.0;
	for (std::size_t i = 0; i < lhs.size(); ++i) {
		cumulative += lhs[i] - rhs[i];
		distance += std::abs(cumulative);
	}
	return distance;
}

auto aligned_curve(const std::vector<double> &lhs, const std::vector<double> &rhs) -> double
{
	constexpr double gap = 2.0;
	std::vector<double> previous(rhs.size() + 1);
	for (std::size_t i = 0; i < previous.size(); ++i) {
		previous[i] = static_cast<double>(i) * gap;
	}
	for (std::size_t i = 1; i <= lhs.size(); ++i) {
		std::vector<double> current(rhs.size() + 1);
		current[0] = static_cast<double>(i) * gap;
		for (std::size_t j = 1; j <= rhs.size(); ++j) {
			const auto sub = previous[j - 1] + std::min(std::abs(lhs[i - 1] - rhs[j - 1]), 2 * gap);
			current[j] = std::min({sub, previous[j] + gap, current[j - 1] + gap});
		}
		previous = std::move(current);
	}
	return previous.back();
}

// Composed domain metric: each field is priced by the distance that matches its
// semantics (numeric L2, exact status match, edit distance on the message, cumulative
// transport on the spectrum, elastic alignment on the curve), then blended.
struct MixedRecordDistance {
	auto operator()(const IndustrialRecord &lhs, const IndustrialRecord &rhs) const -> double
	{
		const auto message_scale = static_cast<double>(std::max({lhs.message.size(), rhs.message.size(), std::size_t{1}}));
		const auto curve_scale = static_cast<double>(std::max({lhs.curve.size(), rhs.curve.size(), std::size_t{1}}));
		return 0.20 * euclidean3(lhs.temperature_summary, rhs.temperature_summary) +
			   0.15 * (lhs.status == rhs.status ? 0.0 : 2.0) +
			   0.15 * static_cast<double>(edit_distance(lhs.message, rhs.message)) / message_scale +
			   0.25 * cumulative_transport(lhs.spectrum, rhs.spectrum) +
			   0.25 * aligned_curve(lhs.curve, rhs.curve) / curve_scale;
	}
};

// Naive baseline: flatten the record to a fixed numeric vector and use Euclidean.
auto flat_projection(const IndustrialRecord &record) -> std::vector<double>
{
	constexpr std::size_t curve_width = 5;
	std::vector<double> result;
	result.insert(result.end(), record.temperature_summary.begin(), record.temperature_summary.end());
	result.insert(result.end(), record.spectrum.begin(), record.spectrum.end());
	for (std::size_t i = 0; i < curve_width; ++i) {
		result.push_back(i < record.curve.size() ? record.curve[i] : 0.0);
	}
	return result;
}

struct FlatVectorDistance {
	auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
	{
		double squared = 0.0;
		for (std::size_t i = 0; i < lhs.size(); ++i) {
			const auto delta = lhs[i] - rhs[i];
			squared += delta * delta;
		}
		return std::sqrt(squared);
	}
};

auto names() -> std::vector<std::string>
{
	return {"warmup-valve-drift",	  "stable-valve-shadow",  "cooldown-pump-cavitation", "warmup-cavitation-shadow",
			"alert-bearing-chatter", "stable-bearing-shadow", "filter-clog-drift",		 "stable-filter-shadow"};
}

auto records() -> std::vector<IndustrialRecord>
{
	return {{{72.0, 73.0, 75.0}, "warmup", "valve drift warning", {0.1, 0.3, 0.4, 0.2}, {1.0, 1.2, 1.5, 1.4, 1.2}},
			{{72.2, 73.1, 75.1}, "stable", "nominal flow", {0.1, 0.3, 0.4, 0.2}, {1.0, 1.2, 1.5, 1.4, 1.3}},
			{{63.8, 63.1, 62.5}, "cooldown", "pump cavitation warning", {0.0, 0.2, 0.55, 0.25}, {2.0, 1.7, 1.3, 1.0}},
			{{64.0, 63.2, 62.7}, "warmup", "warmup normal", {0.0, 0.2, 0.55, 0.25}, {2.0, 1.6, 1.35, 1.05}},
			{{88.0, 92.0, 97.0}, "alert", "bearing chatter alert", {0.0, 0.05, 0.25, 0.7}, {1.0, 5.9, 1.2, 5.8, 1.1}},
			{{88.3, 92.1, 96.8}, "stable", "nominal flow", {0.0, 0.05, 0.25, 0.7}, {1.0, 6.0, 1.2, 5.8, 1.0}},
			{{70.0, 71.0, 72.0}, "degraded", "filter clog warning", {0.3, 0.4, 0.2, 0.1}, {1.5, 1.6, 1.8, 2.1, 2.0}},
			{{70.1, 71.1, 72.2}, "stable", "nominal flow", {0.3, 0.4, 0.2, 0.1}, {1.5, 1.7, 1.9, 2.1, 2.0}}};
}

struct Query {
	std::string expected_name;
	IndustrialRecord record;
};

auto queries() -> std::vector<Query>
{
	return {{"warmup-valve-drift",
			 {{72.2, 73.1, 75.1}, "warmup", "valve drift warning", {0.1, 0.3, 0.4, 0.2}, {1.0, 1.2, 1.5, 1.4, 1.3}}},
			{"cooldown-pump-cavitation",
			 {{64.0, 63.2, 62.7}, "cooldown", "pump cavitation warning", {0.0, 0.2, 0.55, 0.25}, {2.0, 1.6, 1.35, 1.05}}},
			{"alert-bearing-chatter",
			 {{88.3, 92.1, 96.8}, "alert", "bearing chatter alert", {0.0, 0.05, 0.25, 0.7}, {1.0, 6.0, 1.2, 5.8, 1.0}}},
			{"filter-clog-drift",
			 {{70.1, 71.1, 72.2}, "degraded", "filter clog warning", {0.3, 0.4, 0.2, 0.1}, {1.5, 1.7, 1.9, 2.1, 2.0}}}};
}

auto run(BenchmarkReport &report) -> void
{
	const auto record_names = names();
	const auto gallery = records();
	const auto query_set = queries();
	const MixedRecordDistance metric;

	auto space = mtrc::make_space(gallery, metric);
	std::vector<std::vector<double>> flat;
	flat.reserve(gallery.size());
	for (const auto &record : gallery) {
		flat.push_back(flat_projection(record));
	}
	auto baseline = mtrc::make_space(flat, FlatVectorDistance{});

	std::size_t metric_matches = 0;
	std::size_t vector_mismatches = 0;
	double margin_sum = 0.0;
	for (const auto &query : query_set) {
		const auto metric_nn = mtrc::find_neighbors(space, query.record, mtrc::count{1});
		assert(metric_nn.size() == 1);
		assert(record_names[metric_nn[0].id.index()] == query.expected_name);

		const auto vector_nn = mtrc::find_neighbors(baseline, flat_projection(query.record), mtrc::count{1});
		assert(vector_nn.size() == 1);
		assert(record_names[vector_nn[0].id.index()] != query.expected_name);

		const auto vector_pick_under_metric = metric(query.record, gallery[vector_nn[0].id.index()]);
		const auto margin = vector_pick_under_metric - metric_nn[0].distance;
		assert(margin > 0.0);

		++metric_matches;
		++vector_mismatches;
		margin_sum += margin;
	}
	const auto average_margin = margin_sum / static_cast<double>(query_set.size());

	// --- Deterministic evidence ---
	auto reduction = measure_work_reduction("mixed records", "neighbour/relation/summary over composed gallery", gallery,
											metric, 3, "composed domain metric vs flat Euclidean projection");
	report.add_representation_cost(
		mtrc::benchmarks::representation_cost_row("mixed records", reduction.diagnostics, "composed-record all-pairs cache"));
	report.add_performance_row(reduction.row);

	WorkflowEvidenceRow quality;
	quality.benchmark = "mixed records";
	quality.query_count = query_set.size();
	quality.metric_matches = metric_matches;
	quality.vector_mismatches = vector_mismatches;
	quality.average_metric_margin = average_margin;
	quality.reports_average_metric_margin = true;
	quality.diagnostics = "composed-domain-metric-vs-numeric-vector";
	report.add_workflow_evidence(quality);

	// --- Wall-clock evidence (out-of-band) ---
	const std::size_t n = gallery.size();
	const int reps = full_mode() ? 4000 : 40;
	const double metric_ns = median_ns(reps, [&]() {
		double sum = 0.0;
		for (std::size_t i = 0; i < n; ++i)
			for (std::size_t j = 0; j < n; ++j)
				sum += metric(gallery[i], gallery[j]);
		return sum;
	});
	const FlatVectorDistance flat_metric;
	const double baseline_ns = median_ns(reps, [&]() {
		double sum = 0.0;
		for (std::size_t i = 0; i < n; ++i)
			for (std::size_t j = 0; j < n; ++j)
				sum += flat_metric(flat[i], flat[j]);
		return sum;
	});
	const double pairs = static_cast<double>(n * n);
	std::ostringstream line;
	line << std::fixed << std::setprecision(1) << "mixed records      : composed metric "
		 << metric_ns / pairs << " ns/eval  | flat Euclidean " << baseline_ns / pairs << " ns/eval  | cost x"
		 << std::setprecision(2) << (baseline_ns > 0 ? metric_ns / baseline_ns : 0.0);
	record_timing(line.str());
}

} // namespace mixed

// =====================================================================================
//  Hero 2: Condition Monitoring
// =====================================================================================
namespace condition {

using Curve = std::vector<double>;

// Deterministic, platform-independent noise (splitmix64 finalizer in [-1, 1)).
auto hash_unit(std::uint64_t seed) -> double
{
	std::uint64_t z = seed + 0x9E3779B97F4A7C15ULL;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	z = z ^ (z >> 31);
	return static_cast<double>(z >> 11) / 9007199254740992.0 * 2.0 - 1.0;
}

auto jitter(std::uint64_t stream, int position, double scale) -> double
{
	return scale * hash_unit(stream * 1000003ULL + static_cast<std::uint64_t>(position + 1));
}

enum class Regime { Normal, Drift, Fault, Signature };

auto trapezoid(int rise, int ramp, int plateau, int tail, double amplitude, double baseline, std::uint64_t seed) -> Curve
{
	Curve curve;
	int pos = 0;
	for (int i = 0; i < rise; ++i, ++pos)
		curve.push_back(baseline + jitter(seed, pos, 0.02));
	for (int i = 1; i <= ramp; ++i, ++pos)
		curve.push_back(baseline + amplitude * static_cast<double>(i) / ramp + jitter(seed, pos, 0.02));
	for (int i = 0; i < plateau; ++i, ++pos)
		curve.push_back(baseline + amplitude + jitter(seed, pos, 0.03));
	for (int i = ramp - 1; i >= 1; --i, ++pos)
		curve.push_back(baseline + amplitude * static_cast<double>(i) / ramp + jitter(seed, pos, 0.02));
	for (int i = 0; i < tail; ++i, ++pos)
		curve.push_back(baseline + jitter(seed, pos, 0.02));
	return curve;
}

auto healthy(int rise, int ramp, int plateau, int tail, std::uint64_t seed) -> Curve
{
	return trapezoid(rise, ramp, plateau, tail, 1.0, 0.0, seed);
}

auto drift(int step, std::uint64_t seed) -> Curve
{
	return trapezoid(3, 3, 3, 3, 1.0 + 0.14 * step, 0.16 * step, seed);
}

auto fault(std::uint64_t seed) -> Curve
{
	Curve curve;
	int pos = 0;
	for (int i = 0; i < 2; ++i, ++pos)
		curve.push_back(jitter(seed, pos, 0.02));
	for (double value : {1.8, 3.6, 2.0, 3.2, 1.5})
		curve.push_back(value + jitter(seed, pos++, 0.04));
	for (int i = 0; i < 3; ++i, ++pos)
		curve.push_back(jitter(seed, pos, 0.02));
	return curve;
}

auto signature(int rise, int notch_offset, std::uint64_t seed) -> Curve
{
	const int ramp = 3;
	Curve curve = trapezoid(rise, ramp, 6, 3, 1.0, 0.0, seed);
	const int notch_start = rise + ramp + notch_offset;
	for (int i = 0; i < 3; ++i) {
		const int index = notch_start + i;
		if (index >= 0 && index < static_cast<int>(curve.size()))
			curve[index] = 0.0 + jitter(seed, 500 + index, 0.03);
	}
	return curve;
}

// Naive baseline: zero-pad to a common length, compare pointwise (Euclidean window).
struct WindowVectorEuclidean {
	auto operator()(const Curve &lhs, const Curve &rhs) const -> double
	{
		const std::size_t n = std::max(lhs.size(), rhs.size());
		double sum = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			const double a = i < lhs.size() ? lhs[i] : 0.0;
			const double b = i < rhs.size() ? rhs[i] : 0.0;
			sum += (a - b) * (a - b);
		}
		return std::sqrt(sum);
	}
};

auto process_metric() -> mtrc::TWED<double> { return mtrc::TWED<double>(0.5, 0.001); }

struct Catalog {
	std::vector<Curve> curves;
	std::vector<Regime> families;
};

auto build_catalog() -> Catalog
{
	Catalog catalog;
	auto add = [&](Curve curve, Regime regime) {
		catalog.curves.push_back(std::move(curve));
		catalog.families.push_back(regime);
	};
	add(healthy(2, 3, 3, 3, 11), Regime::Normal);
	add(healthy(3, 3, 2, 3, 12), Regime::Normal);
	add(healthy(1, 4, 3, 2, 13), Regime::Normal);
	add(healthy(4, 3, 4, 2, 14), Regime::Normal);
	add(healthy(2, 4, 2, 4, 15), Regime::Normal);
	add(healthy(3, 2, 4, 3, 16), Regime::Normal);
	add(drift(4, 21), Regime::Drift);
	add(drift(5, 22), Regime::Drift);
	add(drift(6, 23), Regime::Drift);
	add(fault(31), Regime::Fault);
	add(fault(32), Regime::Fault);
	add(signature(2, 0, 41), Regime::Signature);
	add(signature(2, 3, 42), Regime::Signature);
	add(signature(3, 1, 43), Regime::Signature);
	return catalog;
}

template <typename Space>
auto classify(const Space &space, const std::vector<Regime> &families, const Curve &query) -> std::pair<Regime, double>
{
	const auto neighbors = mtrc::find_neighbors(space, query, mtrc::count{1});
	assert(!neighbors.empty());
	return {families[neighbors[0].id.index()], neighbors[0].distance};
}

auto run(BenchmarkReport &report) -> void
{
	const auto catalog = build_catalog();
	auto space = mtrc::make_space(catalog.curves, process_metric());
	auto baseline = mtrc::make_space(catalog.curves, WindowVectorEuclidean{});
	const auto &families = catalog.families;

	struct Query {
		Curve curve;
		Regime truth;
	};
	std::vector<Query> queries = {
		{healthy(2, 3, 3, 3, 9001), Regime::Normal},  {healthy(4, 3, 4, 2, 9002), Regime::Normal},
		{healthy(1, 4, 2, 4, 9003), Regime::Normal},  {drift(5, 9101), Regime::Drift},
		{drift(6, 9102), Regime::Drift},			  {fault(9201), Regime::Fault},
		{fault(9202), Regime::Fault},				  {signature(2, 1, 9301), Regime::Signature},
		{signature(3, 4, 9302), Regime::Signature},
	};

	std::size_t twed_correct = 0;
	std::size_t base_correct = 0;
	double margin_sum = 0.0;
	for (const auto &query : queries) {
		const auto [twed_class, twed_dist] = classify(space, families, query.curve);
		const auto [base_class, base_dist] = classify(baseline, families, query.curve);
		(void)base_dist;
		if (twed_class == query.truth)
			++twed_correct;
		if (base_class == query.truth)
			++base_correct;
		const auto base_nn = mtrc::find_neighbors(baseline, query.curve, mtrc::count{1});
		const double base_pick_under_twed = space.metric()(query.curve, catalog.curves[base_nn[0].id.index()]);
		margin_sum += base_pick_under_twed - twed_dist;
	}
	const auto average_margin = margin_sum / static_cast<double>(queries.size());
	assert(twed_correct == queries.size()); // TWED metric space is perfect here
	assert(twed_correct > base_correct);	 // and strictly beats the window-vector baseline
	assert(average_margin > 0.0);

	// --- Deterministic evidence ---
	auto reduction = measure_work_reduction("condition monitoring", "neighbour/outlier/structure over cycle gallery",
											catalog.curves, process_metric(), 3, "TWED elastic metric vs Euclidean window vector");
	report.add_representation_cost(mtrc::benchmarks::representation_cost_row("condition monitoring", reduction.diagnostics,
																			"TWED cycle-curve all-pairs cache"));
	report.add_performance_row(reduction.row);

	WorkflowEvidenceRow quality;
	quality.benchmark = "condition monitoring";
	quality.query_count = queries.size();
	quality.metric_matches = twed_correct;
	quality.vector_mismatches = queries.size() - base_correct;
	quality.average_metric_margin = average_margin;
	quality.reports_average_metric_margin = true;
	quality.diagnostics = "TWED-1NN-accuracy-vs-euclidean-window";
	report.add_workflow_evidence(quality);

	// --- Wall-clock evidence (out-of-band) ---
	const std::size_t n = catalog.curves.size();
	const int reps = full_mode() ? 2000 : 20;
	const auto twed = process_metric();
	const double metric_ns = median_ns(reps, [&]() {
		double sum = 0.0;
		for (std::size_t i = 0; i < n; ++i)
			for (std::size_t j = 0; j < n; ++j)
				sum += twed(catalog.curves[i], catalog.curves[j]);
		return sum;
	});
	const WindowVectorEuclidean window;
	const double baseline_ns = median_ns(reps, [&]() {
		double sum = 0.0;
		for (std::size_t i = 0; i < n; ++i)
			for (std::size_t j = 0; j < n; ++j)
				sum += window(catalog.curves[i], catalog.curves[j]);
		return sum;
	});
	const double pairs = static_cast<double>(n * n);
	std::ostringstream line;
	line << std::fixed << std::setprecision(1) << "condition monitor  : TWED metric    " << metric_ns / pairs
		 << " ns/eval  | Euclidean window " << baseline_ns / pairs << " ns/eval  | cost x" << std::setprecision(2)
		 << (baseline_ns > 0 ? metric_ns / baseline_ns : 0.0);
	record_timing(line.str());
}

} // namespace condition

// =====================================================================================
//  Hero 3: Cross-Space Dependency
// =====================================================================================
namespace cross_space {

auto run(BenchmarkReport &report) -> void
{
	std::vector<std::vector<double>> process_records;
	std::vector<std::array<float, 1>> quality_records;
	for (int index = 0; index < 12; ++index) {
		const auto value = static_cast<double>(index);
		process_records.push_back({value});
		quality_records.push_back({static_cast<float>((value * value) / 10.0)}); // nonlinear monotone dependence
	}

	auto process_space = mtrc::make_space(process_records, mtrc::Euclidean<double>());
	auto quality_space = mtrc::make_space(quality_records, mtrc::Manhattan<float>());

	const auto compared = mtrc::compare(process_space, quality_space, mtrc::stats::correlate::mgc_options{});
	assert(compared.algorithm == "mgc");
	assert(compared.left_record_count == process_space.size());
	assert(compared.right_record_count == quality_space.size());
	assert(compared.exact);
	assert(std::isfinite(compared.value));
	assert(close_to(compared.value, 1.0)); // MGC detects the full monotone dependence

	// Naive baseline: pair each process record to its nearest quality record by raw
	// value. The nonlinear relation defeats it -- only the early indices line up.
	std::size_t raw_correct = 0;
	for (std::size_t i = 0; i < process_records.size(); ++i) {
		std::size_t best = 0;
		double best_distance = std::numeric_limits<double>::infinity();
		for (std::size_t j = 0; j < quality_records.size(); ++j) {
			const double distance = std::abs(process_records[i][0] - static_cast<double>(quality_records[j][0]));
			if (distance < best_distance) {
				best_distance = distance;
				best = j;
			}
		}
		if (best == i)
			++raw_correct;
	}
	const std::size_t raw_mismatches = process_records.size() - raw_correct;
	assert(raw_correct == 3);
	assert(raw_mismatches == 9);

	// --- Deterministic evidence: each space's geometry is consumed by the dependence
	//     test and by structural read-outs; materialize once vs recompute. ---
	auto reduction = measure_work_reduction("cross-space dependency", "process geometry shared by MGC + readouts",
											process_records, mtrc::Euclidean<double>(), 3,
											"MGC dependence vs raw nearest-value pairing");
	report.add_representation_cost(mtrc::benchmarks::representation_cost_row("cross-space dependency", reduction.diagnostics,
																			"process-space distance matrix"));
	report.add_performance_row(reduction.row);

	WorkflowEvidenceRow quality;
	quality.benchmark = "cross-space dependency";
	quality.query_count = process_records.size();
	quality.metric_matches = process_records.size(); // MGC recovers the dependence (score=1)
	quality.vector_mismatches = raw_mismatches;
	quality.diagnostics = "MGC dependence score=1; raw nearest-value pairing correct=3/12";
	report.add_workflow_evidence(quality);

	// --- Wall-clock evidence (out-of-band): cost of the rigorous MGC dependence test. ---
	const int reps = full_mode() ? 400 : 10;
	const double mgc_ns = median_ns(reps, [&]() {
		const auto result = mtrc::compare(process_space, quality_space, mtrc::stats::correlate::mgc_options{});
		return result.value;
	});
	std::ostringstream line;
	line << std::fixed << std::setprecision(1) << "cross-space dep.   : MGC(12x12 spaces) " << mgc_ns / 1000.0
		 << " us/test  | raw pairing is O(n^2) but recovers only 3/12";
	record_timing(line.str());
}

} // namespace cross_space

// =====================================================================================
//  Hero 4: Metric Mapping
// =====================================================================================
namespace mapping {

using record_type = std::vector<double>;

// A curved 1-manifold (half-circle arc with a tiny deterministic radial wobble) in R^2.
auto arc_records(std::size_t count) -> std::vector<record_type>
{
	std::vector<record_type> records;
	records.reserve(count);
	const double pi = 3.14159265358979323846;
	for (std::size_t index = 0; index < count; ++index) {
		const double t = static_cast<double>(index) / static_cast<double>(count - 1);
		const double theta = pi * t;
		const double radius = 1.0 + 0.03 * static_cast<double>(static_cast<int>(index % 3) - 1);
		records.push_back({radius * std::cos(theta), radius * std::sin(theta)});
	}
	return records;
}

template <typename Space> auto mean_offdiagonal_distance(const Space &space) -> double
{
	double total = 0.0;
	std::size_t count = 0;
	for (std::size_t i = 0; i < space.size(); ++i) {
		for (std::size_t j = 0; j < space.size(); ++j) {
			if (i == j)
				continue;
			total += space.distance(space.id(i), space.id(j));
			++count;
		}
	}
	return count == 0 ? 0.0 : total / static_cast<double>(count);
}

auto coordinate_checksum(const mtrc::modify::map::PhateGeometryTargets<double> &targets) -> double
{
	double checksum = 0.0;
	for (const auto &[id, coordinates] : targets.coordinates) {
		(void)id;
		for (const double value : coordinates) {
			checksum += value;
		}
	}
	return checksum;
}

auto run(BenchmarkReport &report) -> void
{
	const std::size_t n = 16;
	const auto records = arc_records(n);
	auto space = mtrc::make_space(records, mtrc::Euclidean<double>());

	mtrc::modify::map::PhateGeometrySpec<double> geometry;
	geometry.dimensions = 1;
	geometry.diffusion_steps = 3;
	geometry.kernel_scale = mean_offdiagonal_distance(space);
	geometry.max_dense_records = records.size();

	const auto targets = mtrc::modify::map::phate_geometry_targets<decltype(space), double>(
		space, geometry, "distance_table_pairwise_distances", "exponential_affinity_kernel",
		"lazy_row_normalized_diffusion_operator");

	// Correctness: the mapping is driven by the metric (dense n^2 evaluations), exact,
	// and produces one anchor coordinate per record.
	assert(targets.dense_distance_evaluations == records.size() * records.size());
	assert(targets.coordinates.size() == records.size());
	assert(targets.dimensions == geometry.dimensions);
	assert(targets.method == "diffusion_potential_anchor_coordinates");

	// Reproducibility: recomputing the diffusion geometry yields byte-identical targets.
	const auto targets_again = mtrc::modify::map::phate_geometry_targets<decltype(space), double>(
		space, geometry, "distance_table_pairwise_distances", "exponential_affinity_kernel",
		"lazy_row_normalized_diffusion_operator");
	const double checksum = coordinate_checksum(targets);
	assert(std::isfinite(checksum));
	assert(coordinate_checksum(targets_again) == checksum);
	assert(targets_again.dense_distance_evaluations == targets.dense_distance_evaluations);

	// --- Deterministic evidence: the source geometry feeds the diffusion target plus
	//     downstream read-outs; materialize once vs recompute. ---
	auto reduction = measure_work_reduction("metric mapping", "source geometry shared by diffusion target + readouts",
											records, mtrc::Euclidean<double>(), 3,
											"PHATE diffusion-geometry targets driven by the source metric");
	report.add_representation_cost(
		mtrc::benchmarks::representation_cost_row("metric mapping", reduction.diagnostics, "arc source-space all-pairs cache"));
	report.add_performance_row(reduction.row);

	WorkflowEvidenceRow quality;
	quality.benchmark = "metric mapping";
	quality.query_count = records.size();
	quality.metric_matches = records.size(); // one metric-anchored coordinate per record
	quality.vector_mismatches = 0;
	quality.latent_matches = records.size();
	quality.reports_latent_matches = true;
	quality.diagnostics = "diffusion-potential anchors: dense_evals=n^2; reproducible (identical coordinates on rerun)";
	report.add_workflow_evidence(quality);

	// --- Wall-clock evidence (out-of-band): diffusion-target construction vs the bare
	//     Euclidean dense matrix the mapping is built on. ---
	const int reps = full_mode() ? 400 : 10;
	const double targets_ns = median_ns(reps, [&]() {
		const auto rebuilt = mtrc::modify::map::phate_geometry_targets<decltype(space), double>(
			space, geometry, "distance_table_pairwise_distances", "exponential_affinity_kernel",
			"lazy_row_normalized_diffusion_operator");
		return static_cast<double>(rebuilt.dense_distance_evaluations);
	});
	const mtrc::Euclidean<double> euclidean;
	const double dense_ns = median_ns(reps, [&]() {
		double sum = 0.0;
		for (std::size_t i = 0; i < n; ++i)
			for (std::size_t j = 0; j < n; ++j)
				sum += euclidean(records[i], records[j]);
		return sum;
	});
	std::ostringstream line;
	line << std::fixed << std::setprecision(1) << "metric mapping     : diffusion targets " << targets_ns / 1000.0
		 << " us/build | bare dense matrix " << dense_ns / 1000.0 << " us/build  (n=" << n << ")";
	record_timing(line.str());
}

} // namespace mapping

auto env_or_default(const char *name, std::string fallback) -> std::string
{
	const auto *value = std::getenv(name);
	if (value != nullptr && value[0] != '\0') {
		return value;
	}
	return fallback;
}

} // namespace

int main()
{
	BenchmarkReport report("METRIC Hero Application Benchmarks");
	report.set_run_metadata(BenchmarkRunMetadata{
		env_or_default("METRIC_HERO_BENCH_SUITE", "hero application evidence suite"),
		env_or_default("METRIC_HERO_BENCH_SOURCE", "examples/engine/hero_application_benchmarks.cpp"),
		env_or_default("METRIC_HERO_BENCH_BUILD_PROFILE", "core preset"),
		env_or_default("METRIC_HERO_BENCH_PLATFORM", "portable deterministic fixture"),
		env_or_default("METRIC_HERO_BENCH_ARTIFACT", "docs/examples/hero-application-benchmarks.md"),
		env_or_default("METRIC_HERO_BENCH_NOTES",
					   "deterministic correctness + work-reduction evidence; wall-clock reported on stderr only")});

	mixed::run(report);
	condition::run(report);
	cross_space::run(report);
	mapping::run(report);

	assert(report.performance_rows().size() == 4);
	assert(report.workflow_evidence().size() == 4);
	assert(report.representation_costs().size() == 4);

	// Byte-stable deterministic artifact -> stdout.
	std::cout << report.to_markdown();

	// Machine-dependent wall-clock -> stderr (never captured into the artifact).
	std::cerr << "\n# Wall-clock timing (machine-dependent; NOT part of the byte-stable artifact)\n";
	std::cerr << "# mode: " << (full_mode() ? "full" : "smoke") << "\n";
	for (const auto &line : g_timing_lines) {
		std::cerr << "  " << line << "\n";
	}
	return 0;
}
