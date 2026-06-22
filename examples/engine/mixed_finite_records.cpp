// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Hero application: "Mixed Finite Metric Records".
//
// Heterogeneous records (text + histogram/image-signal + time series + numeric
// vitals) are made directly comparable by ONE composite true metric, where every
// field is scored by a real admitted METRIC metric. The program demonstrates,
// natively in C++:
//
//   1. the composite is routed as metric_law::metric (cover-tree index accepts it),
//   2. nearest-neighbour search agrees between brute force and the cover tree,
//   3. three per-channel probes where the naive flattened-vector baseline picks
//      the wrong record because it has lost the field's structure,
//   4. representatives (farthest-first fleet archetypes),
//   5. clustering (k-medoids) and its purity vs. the flat baseline,
//   6. outlier / structure analysis (DBSCAN flags the novel fault),
//   7. a cross-space MGC dependence test showing the composite geometry tracks
//      the latent severity outcome far better than the flat vector does.
//
// See docs/examples/mixed-finite-records-hero.md for the recorded output.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include <metric/engine.hpp>

#include "mixed_finite_records.hpp"
#include "mixed_finite_records_fixture.hpp"

namespace {

using hero::FlatEuclidean;
using hero::MixedRecord;
using hero::MixedRecordMetric;

// Always-live check: unlike assert(), this is NOT compiled out under NDEBUG, so
// the demonstrated invariants are verified in every build type.
int g_failures = 0;
#define CHECK(cond)                                                                                                    \
	do {                                                                                                               \
		if (!(cond)) {                                                                                                 \
			std::cerr << "CHECK failed: " << #cond << " (" << __FILE__ << ":" << __LINE__ << ")\n";                   \
			++g_failures;                                                                                              \
		}                                                                                                              \
	} while (0)

// Tuned, fixed, strictly-positive channel weights. They are constants of the
// metric instance (never per-pair normalizers), so the composite stays a true
// metric; see mixed_finite_records.hpp for the proof sketch.
constexpr MixedRecordMetric::Weights kWeights{
	/*code=*/0.6,
	/*spectrum=*/1.0,
	/*curve=*/0.5,
	/*vitals=*/1.0,
};

auto make_fleet_metric(const std::vector<MixedRecord> &records) -> MixedRecordMetric
{
	return MixedRecordMetric(hero::kSpectrumBins, hero::fit_vitals_metric(records), kWeights);
}

auto clustering_purity(const std::vector<std::size_t> &assignments, const std::vector<std::string> &labels,
					   std::size_t cluster_count) -> double
{
	if (assignments.empty()) {
		return 0.0;
	}
	double correct = 0.0;
	for (std::size_t cluster = 0; cluster < cluster_count; ++cluster) {
		std::map<std::string, std::size_t> counts;
		for (std::size_t i = 0; i < assignments.size(); ++i) {
			if (assignments[i] == cluster) {
				++counts[labels[i]];
			}
		}
		std::size_t best = 0;
		for (const auto &entry : counts) {
			best = std::max(best, entry.second);
		}
		correct += static_cast<double>(best);
	}
	return correct / static_cast<double>(assignments.size());
}

} // namespace

int main()
{
	std::cout << std::fixed << std::setprecision(4);

	// -----------------------------------------------------------------------
	// 0. Build the fleet space under the composite true metric.
	// -----------------------------------------------------------------------
	const auto fleet = hero::make_fleet();
	const auto records = hero::fleet_records(fleet);
	const auto metric = make_fleet_metric(records);
	auto space = mtrc::make_space(records, metric);

	std::cout << "=== Mixed Finite Metric Records ===\n";
	std::cout << "fleet records = " << space.size() << "\n";
	std::cout << "composite metric law = " << mtrc::metric_law_name(mtrc::metric_traits<MixedRecordMetric>::law)
			  << "\n";
	std::cout << "composite cache key = " << mtrc::metric_cache_key(metric) << "\n";

	// The cover-tree index only accepts admitted true metrics; building it is a
	// runtime proof that the composite routes as metric_law::metric.
	const auto cover_tree = mtrc::space::storage::cover_tree(space);
	std::cout << "cover-tree accepted composite metric = yes (records = " << cover_tree.record_count() << ")\n";

	// -----------------------------------------------------------------------
	// 1. Channel distance audit (within vs across family).
	// -----------------------------------------------------------------------
	{
		const auto &a = records[0];                                  // family 0, severity 0
		const auto &same_family = records[hero::kSeverityLevels - 1]; // family 0, severity 4
		const auto &other_family = records[hero::kSeverityLevels];    // family 1, severity 0
		const auto within = metric.contributions(a, same_family);
		const auto across = metric.contributions(a, other_family);
		std::cout << "\n--- channel audit (weighted contributions) ---\n";
		std::cout << "within-family  (sev0 vs sev4): code=" << within.code << " spectrum=" << within.spectrum
				  << " curve=" << within.curve << " vitals=" << within.vitals << " total=" << within.total() << "\n";
		std::cout << "across-family  (fam0 vs fam1): code=" << across.code << " spectrum=" << across.spectrum
				  << " curve=" << across.curve << " vitals=" << across.vitals << " total=" << across.total() << "\n";
	}

	// -----------------------------------------------------------------------
	// 2. Search: brute force vs cover tree agree on the nearest fleet record.
	// -----------------------------------------------------------------------
	const auto &query_record = records[2]; // family 0, severity 2
	const auto brute = mtrc::find_neighbors(space, query_record, mtrc::count{3});
	const auto tree = mtrc::find_neighbors(space, query_record, mtrc::count{3}, mtrc::stats::search::cover_tree{});
	std::cout << "\n--- search ---\n";
	std::cout << "brute representation = " << brute.representation << ", cover-tree representation = "
			  << tree.representation << "\n";
	std::cout << "brute nearest = " << fleet[brute[0].id.index()].family << " sev"
			  << fleet[brute[0].id.index()].severity << " at " << brute[0].distance << "\n";
	std::cout << "cover-tree nearest = " << fleet[tree[0].id.index()].family << " sev"
			  << fleet[tree[0].id.index()].severity << " at " << tree[0].distance << "\n";

	// -----------------------------------------------------------------------
	// 3. Per-channel "baseline loses" probes.
	// -----------------------------------------------------------------------
	const auto catalog = hero::make_demo_catalog();
	const auto catalog_metric = make_fleet_metric(catalog.records);
	auto catalog_space = mtrc::make_space(catalog.records, catalog_metric);
	// Probes use the literal naive baseline: flatten every field and run plain
	// Euclidean. This is the genuine "feature vector" default (z-scoring a
	// probability histogram, for instance, is itself unusual). The fair scale-
	// corrected baseline is used for the cross-space MGC headline below.
	auto flat_catalog_space = mtrc::make_space(hero::flat_projection(catalog.records), FlatEuclidean{});

	std::cout << "\n--- per-channel probes (composite vs naive flatten-and-L2 baseline) ---\n";
	std::size_t probe_metric_correct = 0;
	std::size_t probe_flat_wrong = 0;
	for (const auto &probe : catalog.probes) {
		const auto metric_hit = mtrc::find_neighbors(catalog_space, probe.query, mtrc::count{1});
		const auto flat_hit =
			mtrc::find_neighbors(flat_catalog_space, hero::flat_projection(probe.query), mtrc::count{1});
		const auto metric_label = catalog.labels[metric_hit[0].id.index()];
		const auto flat_label = catalog.labels[flat_hit[0].id.index()];

		const auto target_idx = hero::label_index(catalog.labels, probe.target_label);
		const auto decoy_idx = hero::label_index(catalog.labels, probe.decoy_label);
		const auto d_target = catalog_metric.contributions(probe.query, catalog.records[target_idx]);
		const auto d_decoy = catalog_metric.contributions(probe.query, catalog.records[decoy_idx]);

		const bool metric_ok = metric_label == probe.target_label;
		const bool flat_off = flat_label != probe.target_label;
		probe_metric_correct += metric_ok ? 1 : 0;
		probe_flat_wrong += flat_off ? 1 : 0;

		std::cout << "channel " << probe.channel << ":\n";
		std::cout << "    composite picks " << metric_label << " (target=" << probe.target_label
				  << (metric_ok ? " OK)" : " MISS)") << "\n";
		std::cout << "    flat picks      " << flat_label << " (decoy=" << probe.decoy_label
				  << (flat_off ? " -> baseline lost it)" : ")") << "\n";
		std::cout << "    composite d(query,target)=" << d_target.total() << "  d(query,decoy)=" << d_decoy.total()
				  << "\n";
		std::cout << "    why: " << probe.takeaway << "\n";
	}
	std::cout << "probes: composite correct = " << probe_metric_correct << "/" << catalog.probes.size()
			  << ", flat wrong = " << probe_flat_wrong << "/" << catalog.probes.size() << "\n";

	// -----------------------------------------------------------------------
	// 4. Representatives (farthest-first archetypes).
	// -----------------------------------------------------------------------
	const auto reps = mtrc::find_representatives(space, hero::kFamilyCount);
	std::cout << "\n--- representatives (k=" << hero::kFamilyCount << ") ---\n";
	std::cout << "strategy = " << reps.strategy << ", coverage radius = " << reps.coverage_radius
			  << ", avg nearest = " << reps.average_nearest_distance << "\n";
	for (std::size_t i = 0; i < reps.size(); ++i) {
		std::cout << "    rep " << i << " = " << fleet[reps[i].index()].family << " sev"
				  << fleet[reps[i].index()].severity << "\n";
	}

	// -----------------------------------------------------------------------
	// 5. Clustering: composite vs flat baseline purity against family labels.
	// -----------------------------------------------------------------------
	std::vector<std::string> family_labels;
	for (const auto &entry : fleet) {
		family_labels.push_back(entry.family);
	}
	const auto policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	// Both flat baselines: the raw "feature vector" and the fair per-column z-scored
	// one. The headline comparison uses the fair (standardized) baseline.
	const auto fleet_scaler = hero::fit_flat_standardizer(hero::flat_projection(records));
	auto flat_space = mtrc::make_space(hero::standardized_flat_projection(records, fleet_scaler), FlatEuclidean{});
	auto raw_flat_space = mtrc::make_space(hero::flat_projection(records), FlatEuclidean{});

	const auto family_groups =
		mtrc::find_groups(space, mtrc::stats::structural_analysis::k_medoids_options(hero::kFamilyCount), policy);
	const auto flat_family_groups =
		mtrc::find_groups(flat_space, mtrc::stats::structural_analysis::k_medoids_options(hero::kFamilyCount), policy);
	const auto metric_family_purity =
		clustering_purity(family_groups.assignments, family_labels, family_groups.cluster_count);
	const auto flat_family_purity =
		clustering_purity(flat_family_groups.assignments, family_labels, flat_family_groups.cluster_count);

	std::cout << "\n--- clustering (k-medoids, k=" << hero::kFamilyCount << ") ---\n";
	std::cout << "composite: algorithm = " << family_groups.algorithm << ", clusters = " << family_groups.cluster_count
			  << ", family purity = " << metric_family_purity << "\n";
	std::cout << "flat:      clusters = " << flat_family_groups.cluster_count
			  << ", family purity = " << flat_family_purity << "\n";
	std::cout << "(note: family is redundantly encoded -- distinct code letter-multisets, the curve phase, and a\n"
				 " family vital -- so the flat baseline also separates families; the composite's edge is in the\n"
				 " per-channel probes and the severity MGC below, not in family purity.)\n";

	// -----------------------------------------------------------------------
	// 6. Outliers / structure (DBSCAN flags the novel fault).
	// -----------------------------------------------------------------------
	auto records_with_anomaly = records;
	records_with_anomaly.push_back(hero::make_anomaly());
	const auto anomaly_metric = make_fleet_metric(records_with_anomaly);
	auto anomaly_space = mtrc::make_space(records_with_anomaly, anomaly_metric);
	const auto anomaly_index = records_with_anomaly.size() - 1;

	// Distance diagnostics: nearest-neighbour distance per record, used to pick a
	// DBSCAN radius that isolates the anomaly from the dense fleet.
	{
		std::vector<double> nearest(anomaly_space.size(), std::numeric_limits<double>::infinity());
		for (std::size_t i = 0; i < anomaly_space.size(); ++i) {
			for (std::size_t j = 0; j < anomaly_space.size(); ++j) {
				if (i == j) {
					continue;
				}
				nearest[i] = std::min(nearest[i], anomaly_metric(records_with_anomaly[i], records_with_anomaly[j]));
			}
		}
		auto sorted = nearest;
		std::sort(sorted.begin(), sorted.end());
		std::cout << "\n--- distance diagnostics ---\n";
		std::cout << "fleet nearest-neighbour distance: min=" << sorted.front()
				  << " median=" << sorted[sorted.size() / 2] << " max(excl anomaly)=" << sorted[sorted.size() - 2]
				  << "\n";
		std::cout << "anomaly nearest-neighbour distance = " << nearest[anomaly_index] << "\n";
	}

	constexpr double kDbscanRadius = 9.0;
	const auto outliers =
		mtrc::find_outliers(anomaly_space, mtrc::stats::structural_analysis::dbscan_options(kDbscanRadius, 2), policy);
	std::cout << "\n--- outliers / structure (DBSCAN) ---\n";
	std::cout << "strategy = " << outliers.strategy << ", representation = " << outliers.representation
			  << ", flagged = " << outliers.size() << " of " << anomaly_space.size() << "\n";
	bool anomaly_flagged = false;
	for (const auto &outlier : outliers) {
		if (outlier.id.index() == anomaly_index) {
			anomaly_flagged = true;
		}
		std::cout << "    outlier index " << outlier.id.index()
				  << (outlier.id.index() == anomaly_index ? " (injected anomaly)" : "") << "\n";
	}

	// -----------------------------------------------------------------------
	// 7. Cross-space MGC: composite geometry vs flat geometry against outcome.
	// -----------------------------------------------------------------------
	const auto outcome_records = hero::fleet_outcomes(fleet);
	auto outcome_space = mtrc::make_space(outcome_records, mtrc::Euclidean<double>{});
	const auto mgc_metric = mtrc::compare(space, outcome_space, mtrc::stats::correlate::mgc_options{});
	const auto mgc_flat = mtrc::compare(flat_space, outcome_space, mtrc::stats::correlate::mgc_options{});
	const auto mgc_raw_flat = mtrc::compare(raw_flat_space, outcome_space, mtrc::stats::correlate::mgc_options{});

	std::cout << "\n--- cross-space MGC vs latent severity ---\n";
	std::cout << "algorithm = " << mgc_metric.algorithm << "\n";
	std::cout << "composite-space MGC       = " << mgc_metric.value << "\n";
	std::cout << "standardized-flat MGC     = " << mgc_flat.value << " (fair baseline)\n";
	std::cout << "raw-flat MGC              = " << mgc_raw_flat.value << " (un-standardized)\n";
	std::cout << "composite advantage       = " << (mgc_metric.value - mgc_flat.value) << "\n";

	// -----------------------------------------------------------------------
	// Invariants (always-live checks; see CHECK above).
	// -----------------------------------------------------------------------
	CHECK(mtrc::metric_traits<MixedRecordMetric>::law == mtrc::metric_law::metric);
	CHECK(brute[0].id == tree[0].id); // brute force and cover tree agree
	CHECK(probe_metric_correct == catalog.probes.size());
	CHECK(probe_flat_wrong == catalog.probes.size());
	CHECK(reps.size() == hero::kFamilyCount);
	CHECK(metric_family_purity == 1.0);
	CHECK(metric_family_purity >= flat_family_purity);
	CHECK(anomaly_flagged);
	CHECK(mgc_metric.value > mgc_flat.value); // composite beats the fair baseline

	if (g_failures != 0) {
		std::cerr << "mixed_finite_records: " << g_failures << " check(s) failed\n";
		return 1;
	}
	std::cout << "\nall invariants hold\n";
	return 0;
}
