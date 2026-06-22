#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#include "metric/metric/discovery_report.hpp"

namespace {

auto contains_name(const std::vector<mtrc::metric::metric_catalog_entry> &entries, const std::string &name) -> bool
{
	return std::any_of(entries.begin(), entries.end(), [&](const auto &entry) { return entry.name == name; });
}

auto find_name(const std::vector<mtrc::metric::metric_catalog_entry> &entries, const std::string &name)
	-> const mtrc::metric::metric_catalog_entry &
{
	const auto found = std::find_if(entries.begin(), entries.end(), [&](const auto &entry) {
		return entry.name == name;
	});
	assert(found != entries.end());
	return *found;
}

template <typename Metric>
auto assert_entry_matches_registry(const std::vector<mtrc::metric::metric_catalog_entry> &entries,
								   const std::string &name) -> void
{
	const auto &entry = find_name(entries, name);
	assert(entry.admission == mtrc::metric::admission_status_v<Metric>);
	assert(entry.law == mtrc::metric_traits<Metric>::law);
	assert(entry.records == mtrc::metric_traits<Metric>::records);
	assert(entry.discoverable() == mtrc::metric::is_discoverable_metric_v<Metric>);
}

} // namespace

int main()
{
	const auto entries = mtrc::metric::metric_catalog_entries();
	assert(entries.size() >= 35);
	assert(std::all_of(entries.begin(), entries.end(), [](const auto &entry) {
		return entry.discoverable() == mtrc::metric::admission_is_true_metric(entry.admission);
	}));

	assert_entry_matches_registry<mtrc::Euclidean<double>>(entries, "Euclidean");
	assert_entry_matches_registry<mtrc::P_norm<double>>(entries, "P_norm");
	assert_entry_matches_registry<mtrc::Edit<char>>(entries, "Edit");
	assert_entry_matches_registry<mtrc::TWED<double>>(entries, "TWED");
	assert_entry_matches_registry<mtrc::Wasserstein<double>>(entries, "Wasserstein");
	assert_entry_matches_registry<mtrc::Cosine<double>>(entries, "Cosine");
	assert_entry_matches_registry<mtrc::CosineInverted<double>>(entries, "CosineInverted");
	assert_entry_matches_registry<mtrc::Sorensen<double>>(entries, "Sorensen");
	assert_entry_matches_registry<mtrc::EMD<double>>(entries, "EMD");

	assert(std::string(mtrc::metric::record_kind_name(mtrc::record_kind::aligned_vector)) == "aligned_vector");
	assert(std::string(mtrc::record_kind_name(mtrc::record_kind::sequence)) == "sequence");

	const auto vector_report = mtrc::metric::discover_metrics(mtrc::record_kind::aligned_vector);
	assert(vector_report.has_discoverable_metric());
	assert(contains_name(vector_report.entries, "Euclidean"));
	assert(contains_name(vector_report.entries, "WeightedMinkowski"));
	assert(contains_name(vector_report.entries, "BinaryJaccard"));
	assert(contains_name(vector_report.entries, "DiscreteMetric"));
	assert(!contains_name(vector_report.entries, "Cosine"));
	assert(!contains_name(vector_report.entries, "CosineInverted"));
	assert(!contains_name(vector_report.entries, "Sorensen"));

	mtrc::metric::discovery_options include_non_metrics;
	include_non_metrics.include_quarantine = true;
	include_non_metrics.include_rejected = true;
	const auto vector_with_non_metrics = mtrc::metric::discover_metrics(mtrc::record_kind::aligned_vector, include_non_metrics);
	assert(contains_name(vector_with_non_metrics.entries, "Cosine"));
	assert(contains_name(vector_with_non_metrics.entries, "CosineInverted"));
	assert(contains_name(vector_with_non_metrics.entries, "Sorensen"));
	assert(!find_name(vector_with_non_metrics.entries, "Cosine").discoverable());
	assert(!find_name(vector_with_non_metrics.entries, "Sorensen").alternative.empty());

	const auto sequence_report = mtrc::discover_metrics(mtrc::record_kind::sequence);
	assert(contains_name(sequence_report.entries, "Edit"));
	assert(contains_name(sequence_report.entries, "Hamming"));
	assert(contains_name(sequence_report.entries, "ERP"));
	assert(contains_name(sequence_report.entries, "TWED"));
	assert(!contains_name(sequence_report.entries, "Euclidean"));

	const auto structured_report = mtrc::metric::discover_metrics(mtrc::record_kind::structured);
	assert(contains_name(structured_report.entries, "Jaccard"));
	assert(contains_name(structured_report.entries, "Hausdorff"));
	assert(contains_name(structured_report.entries, "Wasserstein"));
	assert(contains_name(structured_report.entries, "TotalVariation"));
	assert(contains_name(structured_report.entries, "EmpiricalCramer"));

	mtrc::metric::discovery_options strict_options;
	strict_options.include_custom_records = false;
	const auto strict_vector = mtrc::metric::discover_metrics(mtrc::record_kind::aligned_vector, strict_options);
	assert(!contains_name(strict_vector.entries, "DiscreteMetric"));

	return 0;
}
