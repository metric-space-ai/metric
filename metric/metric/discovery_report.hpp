// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_METRIC_DISCOVERY_REPORT_HPP
#define _METRIC_METRIC_DISCOVERY_REPORT_HPP

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <metric/core/metric_traits.hpp>
#include <metric/metric/discovery.hpp>

namespace mtrc::metric {

inline auto record_kind_name(core::record_kind kind) -> const char *
{
	switch (kind) {
	case core::record_kind::custom:
		return "custom";
	case core::record_kind::vector:
		return "vector";
	case core::record_kind::aligned_vector:
		return "aligned_vector";
	case core::record_kind::string:
		return "string";
	case core::record_kind::sequence:
		return "sequence";
	case core::record_kind::structured:
		return "structured";
	}
	return "custom";
}

struct metric_catalog_entry {
	std::string name;
	core::record_kind records{core::record_kind::custom};
	admission_status admission{admission_status::quarantine};
	core::metric_law law{core::metric_law::unknown};
	std::string domain;
	std::string gate;
	std::string recoding;
	std::string alternative;

	auto discoverable() const -> bool { return admission_is_true_metric(admission); }
};

template <typename Metric>
auto make_metric_catalog_entry(std::string name, std::string domain, std::string gate, std::string recoding,
							   std::string alternative = {}) -> metric_catalog_entry
{
	return metric_catalog_entry{std::move(name),
								core::metric_traits<Metric>::records,
								admission_status_v<Metric>,
								core::metric_traits<Metric>::law,
								std::move(domain),
								std::move(gate),
								std::move(recoding),
								std::move(alternative)};
}

struct discovery_options {
	bool include_quarantine{false};
	bool include_rejected{false};
	bool include_custom_records{true};
};

struct metric_discovery_report {
	core::record_kind requested_records{core::record_kind::custom};
	std::vector<metric_catalog_entry> entries;
	std::vector<metric_catalog_entry> alternatives;

	auto empty() const -> bool { return entries.empty(); }
	auto size() const -> std::size_t { return entries.size(); }
	auto has_discoverable_metric() const -> bool
	{
		return std::any_of(entries.begin(), entries.end(), [](const auto &entry) { return entry.discoverable(); });
	}
};

inline auto metric_catalog_entries() -> std::vector<metric_catalog_entry>
{
	std::vector<metric_catalog_entry> entries;
	entries.reserve(41);

	entries.push_back(make_metric_catalog_entry<::mtrc::Euclidean<double>>(
		"Euclidean", "aligned finite real vectors", "same dimension, finite values",
		"L2 coordinate displacement"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Manhattan<double>>(
		"Manhattan", "aligned finite real vectors", "same dimension, finite values",
		"L1 coordinate displacement"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Chebyshev<double>>(
		"Chebyshev", "aligned finite real vectors", "same dimension, finite values",
		"maximum coordinate displacement"));
	entries.push_back(make_metric_catalog_entry<::mtrc::P_norm<double>>(
		"P_norm", "aligned finite real vectors", "finite exponent p >= 1",
		"Lp coordinate displacement"));
	entries.push_back(make_metric_catalog_entry<::mtrc::WeightedMinkowski<double>>(
		"WeightedMinkowski", "aligned finite real vectors", "positive weights, finite p >= 1",
		"weighted Lp coordinate displacement"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Mahalanobis<double>>(
		"Mahalanobis", "aligned finite real vectors", "symmetric positive definite precision matrix",
		"SPD-normalized coordinate displacement"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Euclidean_standardized<double>>(
		"Euclidean_standardized", "aligned finite real vectors", "positive fitted scale in every coordinate",
		"L2 displacement after positive scale normalization"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Manhattan_standardized<double>>(
		"Manhattan_standardized", "aligned finite real vectors", "positive fitted scale in every coordinate",
		"L1 displacement after positive scale normalization"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Angular<double>>(
		"Angular", "unit finite vectors / directions", "non-empty unit vectors, no zero vector",
		"geodesic angle between directions"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Chordal<double>>(
		"Chordal", "unit finite vectors / directions", "non-empty unit vectors, no zero vector",
		"Euclidean chord between directions"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Ruzicka<double>>(
		"Ruzicka", "nonnegative aligned vectors", "finite nonnegative values, same dimension",
		"weighted-set non-overlap"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Tanimoto<double>>(
		"Tanimoto", "nonnegative aligned weighted sets", "finite nonnegative values, same dimension",
		"admitted weighted-Jaccard/Ruzicka computation"));
	entries.push_back(make_metric_catalog_entry<::mtrc::BinaryJaccard>(
		"BinaryJaccard", "aligned binary presence vectors", "entries exactly 0 or 1",
		"set-membership disagreement over aligned binary records"));

	entries.push_back(make_metric_catalog_entry<::mtrc::Edit<char>>(
		"Edit", "strings and symbolic sequences", "finite sequences",
		"minimal symbol rewrite cost"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Hamming>(
		"Hamming", "equal-length categorical or binary sequences", "same length",
		"count of aligned symbol substitutions"));
	entries.push_back(make_metric_catalog_entry<::mtrc::ERP<double>>(
		"ERP", "finite scalar time series", "finite gap value outside the sample alphabet",
		"gap-aware edit cost over scalar sequences"));
	entries.push_back(make_metric_catalog_entry<::mtrc::TWED<double>>(
		"TWED", "finite real time series", "penalty >= 0, elastic > 0, finite values",
		"elastic edit cost with temporal penalty"));

	entries.push_back(make_metric_catalog_entry<::mtrc::DiscreteMetric<double>>(
		"DiscreteMetric", "equality-comparable records", "positive mismatch cost",
		"flat positive cost for different records"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Jaccard>(
		"Jaccard", "finite set records", "finite set semantics; empty/empty fixed at zero",
		"fractional set non-overlap"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Hausdorff<>>(
		"Hausdorff", "non-empty finite point sets", "non-empty sets and admitted ground metric",
		"worst nearest-neighbor set mismatch"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Wasserstein<double>>(
		"Wasserstein", "equal-mass finite measures", "equal nonnegative mass and metric ground cost",
		"minimum mass-transport cost"));
	entries.push_back(make_metric_catalog_entry<::mtrc::TotalVariation<double>>(
		"TotalVariation", "aligned normalized probability vectors", "nonnegative probabilities summing to 1",
		"half-L1 probability disagreement"));
	entries.push_back(make_metric_catalog_entry<::mtrc::Hellinger<double>>(
		"Hellinger", "aligned normalized probability vectors", "nonnegative probabilities summing to 1",
		"Euclidean displacement between square-root densities"));
	entries.push_back(make_metric_catalog_entry<::mtrc::SqrtJensenShannon<double>>(
		"SqrtJensenShannon", "aligned normalized probability vectors", "nonnegative probabilities summing to 1",
		"square root of Jensen-Shannon divergence"));
	entries.push_back(make_metric_catalog_entry<::mtrc::EmpiricalKolmogorovSmirnov<double>>(
		"EmpiricalKolmogorovSmirnov", "finite empirical distributions", "non-empty finite samples",
		"supremum gap between empirical step CDFs"));
	entries.push_back(make_metric_catalog_entry<::mtrc::EmpiricalCramer<double>>(
		"EmpiricalCramer", "finite empirical distributions", "non-empty finite samples",
		"root-L2 gap between empirical step CDFs"));

	entries.push_back(make_metric_catalog_entry<::mtrc::Cosine<double>>(
		"Cosine", "raw vectors", "quarantined: zero vector and scalar-multiple collapse",
		"direction-like score under an ambiguous public name", "Use Angular or Chordal under unit-vector gates."));
	entries.push_back(make_metric_catalog_entry<::mtrc::CosineInverted<double>>(
		"CosineInverted", "raw vectors", "rejected: raw cosine complement is not a metric",
		"similarity complement", "Use Angular or Chordal under unit-vector gates."));
	entries.push_back(make_metric_catalog_entry<::mtrc::Sorensen<double>>(
		"Sorensen", "counts, sets, or nonnegative vectors", "rejected: Bray-Curtis/Sorensen violates triangle law",
		"similarity complement", "Use Jaccard, Ruzicka, Tanimoto, Hamming, or TotalVariation where domains match."));
	entries.push_back(make_metric_catalog_entry<::mtrc::EMD<double>>(
		"EMD", "histograms / measures", "quarantined: arbitrary ground cost and unbalanced mass are permitted",
		"permissive transport compatibility distance", "Use strict Wasserstein."));
	entries.push_back(make_metric_catalog_entry<::mtrc::RandomEMD<std::vector<double>, double>>(
		"RandomEMD", "sampled distributions", "quarantined: approximation route is not admitted as a metric",
		"Wasserstein-like approximation", "Use Wasserstein or empirical CDF metrics."));
	entries.push_back(make_metric_catalog_entry<::mtrc::CramervonMises<std::vector<double>, double>>(
		"CramervonMises", "sampled distributions", "quarantined: Akima-CDF route is not admitted",
		"root-CDF approximation", "Use EmpiricalCramer."));
	entries.push_back(make_metric_catalog_entry<::mtrc::KolmogorovSmirnov<std::vector<double>, double>>(
		"KolmogorovSmirnov", "sampled distributions", "quarantined: Akima-CDF route is not admitted",
		"finite-grid CDF gap", "Use EmpiricalKolmogorovSmirnov."));
	entries.push_back(make_metric_catalog_entry<::mtrc::Weierstrass<double>>(
		"Weierstrass", "vectors / hyperbolic model candidates", "quarantined: model-domain and acosh guard missing",
		"hyperbolic displacement candidate", "Admit a guarded Hyperbolic metric variant."));
	entries.push_back(make_metric_catalog_entry<::mtrc::Euclidean_thresholded<double>>(
		"Euclidean_thresholded", "aligned numeric vectors", "quarantined: positive parameter gates/proof missing",
		"bounded Euclidean displacement candidate", "Admit a guarded TruncatedEuclidean metric variant."));
	entries.push_back(make_metric_catalog_entry<::mtrc::Euclidean_hard_clipped<double>>(
		"Euclidean_hard_clipped", "aligned numeric vectors", "quarantined: positive parameter gates/proof missing",
		"bounded Euclidean displacement candidate", "Admit a guarded TruncatedEuclidean metric variant."));
	entries.push_back(make_metric_catalog_entry<::mtrc::Euclidean_soft_clipped<double>>(
		"Euclidean_soft_clipped", "aligned numeric vectors", "quarantined: concavity and parameter gates not admitted",
		"saturating Euclidean displacement candidate", "Admit a guarded SaturatingEuclidean metric variant."));
	entries.push_back(make_metric_catalog_entry<::mtrc::Hassanat<double>>(
		"Hassanat", "numeric vectors", "quarantined: shipped negative branch is not admitted",
		"branchy numeric dissimilarity", "Fix and admit the published Hassanat metric variant, or keep quarantined."));

	return entries;
}

inline auto include_entry(const metric_catalog_entry &entry, discovery_options options) -> bool
{
	if (entry.discoverable()) {
		return true;
	}
	if (entry.admission == admission_status::quarantine) {
		return options.include_quarantine;
	}
	if (entry.admission == admission_status::rejected) {
		return options.include_rejected;
	}
	return false;
}

inline auto matches_record_kind(const metric_catalog_entry &entry, core::record_kind requested,
								discovery_options options) -> bool
{
	if (entry.records == requested) {
		return true;
	}
	return options.include_custom_records && entry.records == core::record_kind::custom;
}

inline auto discover_metrics(core::record_kind records, discovery_options options = {}) -> metric_discovery_report
{
	metric_discovery_report report;
	report.requested_records = records;
	for (const auto &entry : metric_catalog_entries()) {
		if (!include_entry(entry, options)) {
			continue;
		}
		if (matches_record_kind(entry, records, options)) {
			report.entries.push_back(entry);
		} else if (!entry.discoverable() && !entry.alternative.empty()) {
			report.alternatives.push_back(entry);
		}
	}
	return report;
}

} // namespace mtrc::metric

namespace mtrc {
using metric::discover_metrics;
using metric::discovery_options;
using metric::metric_catalog_entries;
using metric::metric_catalog_entry;
using metric::metric_discovery_report;
using metric::record_kind_name;
} // namespace mtrc

#endif
