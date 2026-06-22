// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Metric-admission proof for the "Mixed Finite Metric Records" hero composite.
//
// The composite distance is a strictly-positive-weighted sum of four real,
// admitted METRIC true metrics over a product domain:
//
//   D(x, y) = w_code * Edit(code) + w_spectrum * Wasserstein(spectrum)
//           + w_curve * TWED(curve) + w_vitals * Euclidean_standardized(vitals)
//
// A conic combination of true metrics with strictly positive weights is itself a
// true metric, so the type honestly declares metric_law::metric. This file is the
// CI gate for that claim. It checks, for the composite:
//
//   1. trait promotion (metric_law::metric, record_kind::structured, thread-safe),
//   2. the metric axioms (non-negativity, symmetry, identity of indiscernibles,
//      triangle inequality) over a hand-built finite domain AND a deterministic
//      randomized property search (the random search includes per-field identity
//      probes so a future ground-cost regression to a pseudometric is caught),
//   3. the admission gate -- strictly positive finite weights are required, and
//      every per-field metric guard (equal-mass spectra, non-empty finite curves)
//      propagates through the composite,
//   4. metric-only routing -- the cover-tree index, which rejects non-metrics,
//      accepts the composite,
//   5. the hero regression invariants -- the three per-channel baseline-loses
//      probes and the cross-space MGC advantage (against a fair standardized
//      baseline) stay true.
//
// Verification uses an always-live CHECK (not assert), so the proof is never
// silently compiled out under NDEBUG. The composite type and fixture are the same
// headers the runnable example uses, so this proof and the demo cannot drift apart.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/engine.hpp"
#include "metric/metric/admission.hpp"

#include "examples/engine/mixed_finite_records.hpp"
#include "examples/engine/mixed_finite_records_fixture.hpp"

namespace {

using hero::FlatEuclidean;
using hero::MixedRecord;
using hero::MixedRecordMetric;

int g_failures = 0;
#define CHECK(cond)                                                                                                    \
	do {                                                                                                               \
		if (!(cond)) {                                                                                                 \
			std::fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__);                             \
			++g_failures;                                                                                              \
		}                                                                                                              \
	} while (0)

constexpr double kEps = 1e-7;

// A representative composite instance: explicit positive standardized scale that
// mirrors the fleet's coordinate magnitudes (mean cancels, so it is arbitrary).
auto make_probe_metric() -> MixedRecordMetric
{
	const mtrc::Euclidean_standardized<double> vitals({0.0, 0.0, 0.0, 0.0}, {2.0, 500.0, 1.0, 1.0});
	return MixedRecordMetric(hero::kSpectrumBins, vitals, MixedRecordMetric::Weights{0.6, 1.0, 0.5, 1.0});
}

// ---------------------------------------------------------------------------
// Metric axiom battery (close variant: distances are real-valued).
// ---------------------------------------------------------------------------

template <typename Metric>
void check_metric_contracts(const std::vector<MixedRecord> &records, const Metric &metric)
{
	for (const auto &a : records) {
		CHECK(std::abs(metric(a, a)) <= kEps); // identity: d(a, a) = 0

		for (const auto &b : records) {
			const double dab = metric(a, b);
			const double dba = metric(b, a);

			CHECK(dab >= -kEps);                // non-negativity
			CHECK(std::abs(dab - dba) <= kEps); // symmetry

			if (dab <= kEps) {
				CHECK(a == b); // identity of indiscernibles
			}

			for (const auto &c : records) {
				CHECK(metric(a, c) <= dab + metric(b, c) + kEps); // triangle inequality
			}
		}
	}
}

// Deterministic xorshift so the randomized property search is reproducible.
struct Rng {
	std::uint64_t s = 0x2545F4914F6CDD1Dull;
	auto next() -> std::uint64_t
	{
		s ^= s << 13;
		s ^= s >> 7;
		s ^= s << 17;
		return s;
	}
	auto below(int n) -> int { return static_cast<int>(next() % static_cast<std::uint64_t>(n)); }
	auto unit() -> double { return static_cast<double>(next() % 1000u) / 1000.0; }
};

auto random_record(Rng &rng) -> MixedRecord
{
	MixedRecord record;

	const int code_length = 1 + rng.below(6);
	for (int i = 0; i < code_length; ++i) {
		record.code.push_back(static_cast<char>('a' + rng.below(4)));
	}

	std::vector<double> bins(hero::kSpectrumBins, 0.0);
	for (auto &value : bins) {
		value = rng.unit();
	}
	bins[static_cast<std::size_t>(rng.below(static_cast<int>(hero::kSpectrumBins)))] += 1.0; // guarantee mass > 0
	record.spectrum = hero::unit_mass(std::move(bins));

	const int curve_length = 1 + rng.below(6);
	record.curve.resize(static_cast<std::size_t>(curve_length));
	for (auto &value : record.curve) {
		value = static_cast<double>(rng.below(5));
	}

	record.vitals = {rng.unit() * 3.0, rng.unit() * 1000.0, rng.unit() * 5.0, rng.unit() * 5.0};
	return record;
}

template <typename Metric> void check_random_contracts(const Metric &metric, int trials)
{
	Rng rng;
	for (int t = 0; t < trials; ++t) {
		const auto a = random_record(rng);
		const auto b = random_record(rng);
		const auto c = random_record(rng);

		const double dab = metric(a, b);
		const double dbc = metric(b, c);
		const double dac = metric(a, c);
		const double dba = metric(b, a);

		const double scale = std::max({1.0, std::abs(dab), std::abs(dbc), std::abs(dac)});
		CHECK(dab >= -kEps);                        // non-negativity
		CHECK(std::abs(dab - dba) <= kEps * scale); // symmetry
		CHECK(dac <= dab + dbc + kEps * scale);     // triangle inequality

		// Identity of indiscernibles, probed per field: a single-field change must
		// move the composite distance off zero, so a future weakening of any ground
		// cost to a pseudometric (e.g. a zero off-diagonal) is caught here too.
		CHECK(std::abs(metric(a, a)) <= kEps); // d(a, a) = 0

		MixedRecord code_mut = a;
		code_mut.code.push_back('z');
		CHECK(metric(a, code_mut) > kEps);

		MixedRecord spectrum_mut = a;
		{
			// Move a unit of mass between two bins -> a genuinely different histogram.
			std::vector<double> bins = a.spectrum;
			bins.front() += 1.0;
			spectrum_mut.spectrum = hero::unit_mass(std::move(bins));
			if (spectrum_mut.spectrum != a.spectrum) {
				CHECK(metric(a, spectrum_mut) > kEps);
			}
		}

		MixedRecord curve_mut = a;
		curve_mut.curve.front() += 1.0;
		CHECK(metric(a, curve_mut) > kEps);

		MixedRecord vitals_mut = a;
		vitals_mut.vitals.front() += 1.0;
		CHECK(metric(a, vitals_mut) > kEps);
	}
}

template <typename Callable> auto throws_invalid_argument(Callable &&call) -> bool
{
	try {
		call();
	} catch (const std::invalid_argument &) {
		return true;
	} catch (...) {
		return false;
	}
	return false;
}

auto make_simple_record(const std::string &code, std::size_t peak, std::vector<double> curve,
						std::vector<double> vitals) -> MixedRecord
{
	MixedRecord record;
	record.code = code;
	record.spectrum = hero::peak_spectrum(peak);
	record.curve = std::move(curve);
	record.vitals = std::move(vitals);
	return record;
}

} // namespace

// 1. Trait promotion: the composite is an admitted true metric.
static_assert(mtrc::metric_traits<MixedRecordMetric>::law == mtrc::metric_law::metric);
static_assert(mtrc::metric_traits<MixedRecordMetric>::records == mtrc::record_kind::structured);
static_assert(mtrc::metric_thread_safe_v<MixedRecordMetric>);
static_assert(mtrc::metric::is_admitted_metric_v<MixedRecordMetric>);

int main()
{
	const auto metric = make_probe_metric();

	// -----------------------------------------------------------------------
	// 2a. Metric axioms over a hand-built finite domain (all four fields vary).
	// -----------------------------------------------------------------------
	const std::vector<MixedRecord> domain = {
		make_simple_record("valve", 3, {1.0, 2.0, 1.0}, {0.0, 100.0, 3.0, 1.0}),
		make_simple_record("valve", 5, {1.0, 2.0, 1.0}, {1.0, 100.0, 3.0, 2.0}),
		make_simple_record("valve", 3, {1.0, 2.0, 3.0, 2.0, 1.0}, {0.0, 100.0, 5.0, 1.0}),
		make_simple_record("bearing", 3, {1.0, 2.0, 1.0}, {2.0, 600.0, 3.0, 4.0}),
		make_simple_record("bearing", 9, {0.5, 0.5}, {3.0, 600.0, 2.0, 5.0}),
		make_simple_record("pump", 7, {3.0, 2.0, 1.0}, {1.5, 1100.0, 4.0, 0.5}),
		make_simple_record("filter", 11, {1.0}, {0.5, 1600.0, 1.0, 2.5}),
		make_simple_record("filterbed", 11, {1.0, 1.0, 1.0, 1.0}, {2.5, 1600.0, 6.0, 3.5}),
	};
	check_metric_contracts(domain, metric);

	// Distinct records must keep d > 0 (identity of indiscernibles really needs
	// every field, i.e. every weight strictly positive).
	for (std::size_t i = 0; i < domain.size(); ++i) {
		for (std::size_t j = i + 1; j < domain.size(); ++j) {
			CHECK(domain[i] != domain[j]);
			CHECK(metric(domain[i], domain[j]) > kEps);
		}
	}

	// -----------------------------------------------------------------------
	// 2b. Deterministic randomized property search (incl. per-field identity).
	// -----------------------------------------------------------------------
	check_random_contracts(metric, 6000);
	check_random_contracts(MixedRecordMetric(hero::kSpectrumBins,
											 mtrc::Euclidean_standardized<double>({0, 0, 0, 0}, {1, 1, 1, 1}),
											 MixedRecordMetric::Weights{1.0, 1.0, 1.0, 1.0}),
						   6000);

	// -----------------------------------------------------------------------
	// 3. Admission gate: strictly positive finite weights are mandatory.
	// -----------------------------------------------------------------------
	const mtrc::Euclidean_standardized<double> any_vitals({0, 0, 0, 0}, {1, 1, 1, 1});
	const auto reject_weights = [&](MixedRecordMetric::Weights w) {
		return throws_invalid_argument([&] { (void)MixedRecordMetric(hero::kSpectrumBins, any_vitals, w); });
	};
	CHECK(reject_weights({0.0, 1.0, 1.0, 1.0}));  // zero weight -> would demote to pseudometric
	CHECK(reject_weights({-1.0, 1.0, 1.0, 1.0})); // negative weight
	CHECK(reject_weights({1.0, 0.0, 1.0, 1.0}));
	CHECK(reject_weights({1.0, 1.0, std::nan(""), 1.0}));
	CHECK(reject_weights({1.0, 1.0, 1.0, std::numeric_limits<double>::infinity()}));

	// Per-field metric guards propagate through the composite.
	MixedRecord unit = make_simple_record("valve", 3, {1.0, 2.0, 1.0}, {1.0, 100.0, 3.0, 2.0});
	MixedRecord bad_mass = unit;
	bad_mass.spectrum = {0.5, 0.5, 0.5}; // wrong length AND not unit mass
	CHECK(throws_invalid_argument([&] { (void)metric(unit, bad_mass); }));
	MixedRecord empty_curve = unit;
	empty_curve.curve = {};
	CHECK(throws_invalid_argument([&] { (void)metric(unit, empty_curve); })); // TWED rejects empty sequence

	// The trait cache key carries the full metric identity (including the fitted
	// vitals scale), so composites that differ only in sigma do NOT collide.
	const MixedRecordMetric sigma_a(hero::kSpectrumBins,
									mtrc::Euclidean_standardized<double>({0, 0, 0, 0}, {2, 500, 1, 1}),
									MixedRecordMetric::Weights{0.6, 1.0, 0.5, 1.0});
	const MixedRecordMetric sigma_b(hero::kSpectrumBins,
									mtrc::Euclidean_standardized<double>({0, 0, 0, 0}, {1, 1, 1, 1}),
									MixedRecordMetric::Weights{0.6, 1.0, 0.5, 1.0});
	CHECK(mtrc::metric_cache_key(sigma_a) != mtrc::metric_cache_key(sigma_b));

	// -----------------------------------------------------------------------
	// 4. Metric-only routing: the cover-tree index accepts the composite.
	// -----------------------------------------------------------------------
	auto domain_space = mtrc::make_space(domain, metric);
	const auto tree = mtrc::space::storage::cover_tree(domain_space);
	CHECK(tree.record_count() == domain.size());
	const auto knn = mtrc::find_neighbors(domain_space, domain[1], mtrc::count{1}, mtrc::stats::search::cover_tree{});
	CHECK(knn.representation == "cover_tree_index");
	CHECK(knn.size() == 1);

	// -----------------------------------------------------------------------
	// 5. Hero regression invariants (shared fixture).
	// -----------------------------------------------------------------------
	const auto fleet = hero::make_fleet();
	const auto records = hero::fleet_records(fleet);
	const MixedRecordMetric fleet_metric(hero::kSpectrumBins, hero::fit_vitals_metric(records),
										 MixedRecordMetric::Weights{0.6, 1.0, 0.5, 1.0});
	auto fleet_space = mtrc::make_space(records, fleet_metric);

	// Brute force and the cover tree agree on the nearest record.
	const auto &query = records[2];
	const auto brute = mtrc::find_neighbors(fleet_space, query, mtrc::count{1});
	const auto routed = mtrc::find_neighbors(fleet_space, query, mtrc::count{1}, mtrc::stats::search::cover_tree{});
	CHECK(brute[0].id == routed[0].id);

	// Each per-channel probe: composite picks the structurally correct target,
	// the naive flatten-and-L2 baseline picks the decoy it cannot tell apart.
	const auto catalog = hero::make_demo_catalog();
	const MixedRecordMetric catalog_metric(hero::kSpectrumBins, hero::fit_vitals_metric(catalog.records),
										   MixedRecordMetric::Weights{0.6, 1.0, 0.5, 1.0});
	auto catalog_space = mtrc::make_space(catalog.records, catalog_metric);
	auto flat_catalog_space = mtrc::make_space(hero::flat_projection(catalog.records), FlatEuclidean{});

	for (const auto &probe : catalog.probes) {
		const auto metric_hit = mtrc::find_neighbors(catalog_space, probe.query, mtrc::count{1});
		const auto flat_hit =
			mtrc::find_neighbors(flat_catalog_space, hero::flat_projection(probe.query), mtrc::count{1});
		CHECK(catalog.labels[metric_hit[0].id.index()] == probe.target_label);
		CHECK(catalog.labels[flat_hit[0].id.index()] == probe.decoy_label);
	}

	// Cross-space MGC: the composite geometry tracks the latent severity outcome
	// strictly better than the FAIR (per-column standardized) flat baseline.
	const auto outcomes = hero::fleet_outcomes(fleet);
	auto outcome_space = mtrc::make_space(outcomes, mtrc::Euclidean<double>{});
	const auto scaler = hero::fit_flat_standardizer(hero::flat_projection(records));
	auto flat_fleet_space = mtrc::make_space(hero::standardized_flat_projection(records, scaler), FlatEuclidean{});
	const auto mgc_metric = mtrc::compare(fleet_space, outcome_space, mtrc::stats::correlate::mgc_options{});
	const auto mgc_flat = mtrc::compare(flat_fleet_space, outcome_space, mtrc::stats::correlate::mgc_options{});
	CHECK(std::isfinite(mgc_metric.value));
	CHECK(mgc_metric.value > mgc_flat.value);

	// The novel fault is isolated as the only DBSCAN outlier.
	auto with_anomaly = records;
	with_anomaly.push_back(hero::make_anomaly());
	const MixedRecordMetric anomaly_metric(hero::kSpectrumBins, hero::fit_vitals_metric(with_anomaly),
										   MixedRecordMetric::Weights{0.6, 1.0, 0.5, 1.0});
	auto anomaly_space = mtrc::make_space(with_anomaly, anomaly_metric);
	const auto policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto outliers =
		mtrc::find_outliers(anomaly_space, mtrc::stats::structural_analysis::dbscan_options(9.0, 2), policy);
	CHECK(outliers.size() == 1);
	CHECK(outliers[0].id.index() == with_anomaly.size() - 1);

	if (g_failures != 0) {
		std::fprintf(stderr, "metric_mixed_records_smoke: %d check(s) failed\n", g_failures);
		return 1;
	}
	return 0;
}
