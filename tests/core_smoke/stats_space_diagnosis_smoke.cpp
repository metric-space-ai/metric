#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/workflow.hpp"

namespace {

template <typename Exception, typename Function> auto throws(Function run) -> bool
{
	try {
		run();
	} catch (const Exception &) {
		return true;
	}
	return false;
}

} // namespace

int main()
{
	using record_type = std::string;
	using metric_type = mtrc::Edit<char>;

	const std::vector<record_type> records{"pump_ok", "pump_warn", "pump_fail", "valve_ok"};
	const auto space = mtrc::space::build_checked(records, metric_type{});

	mtrc::stats::diagnose_options options;
	options.profile.include_distance_distribution = true;
	options.profile.distribution.bucket_count = 4;
	options.profile.include_local_volume = true;
	options.profile.local_volume_radius = 4.0;
	options.neighbor_count = 2;
	options.outlier_neighbor_count = 1;

	const auto report = mtrc::stats::diagnose_space(space, options);
	assert(report.algorithm == "diagnose_space");
	assert(report.representation == "metric_space");
	assert(report.metric_law == mtrc::metric_law::metric);
	assert(report.admission_status == mtrc::metric::admission_status::admitted);
	assert(report.discoverable_metric);
	assert(report.exact);
	assert(report.notes.empty());

	assert(report.profile.record_count == records.size());
	assert(report.profile.pair_count == 6);
	assert(report.profile.has_distance_distribution);
	assert(report.profile.distance_distribution.bucket_count() == 4);
	assert(report.profile.has_local_volume);

	assert(report.has_neighbor_check);
	assert(report.neighbor_check.operator_name == "knn");
	assert(report.neighbor_check.requested_count == 2);
	assert(report.neighbor_check.size() == 2);
	assert(report.neighbor_check[0].id != space.id(0));

	assert(report.has_outlier_scores);
	assert(report.outliers.operator_name == "find_outliers");
	assert(report.outliers.strategy == "nearest_neighbor_distance");
	assert(report.outliers.size() == records.size());

	mtrc::stats::diagnose_options no_sections;
	no_sections.include_neighbor_check = false;
	no_sections.include_outlier_scores = false;
	const auto compact = mtrc::diagnose_space(space, no_sections);
	assert(compact.profile.record_count == records.size());
	assert(!compact.has_neighbor_check);
	assert(!compact.has_outlier_scores);

	const auto empty_space = mtrc::space::build(std::vector<record_type>{}, metric_type{});
	const auto empty_report = mtrc::stats::diagnose_space(empty_space);
	assert(empty_report.profile.is_empty);
	assert(!empty_report.has_neighbor_check);
	assert(!empty_report.has_outlier_scores);
	assert(empty_report.notes.size() == 1);

	const auto singleton_space = mtrc::space::build(std::vector<record_type>{"only"}, metric_type{});
	const auto singleton_report = mtrc::stats::diagnose_space(singleton_space);
	assert(singleton_report.profile.is_singleton);
	assert(!singleton_report.has_neighbor_check);
	assert(!singleton_report.has_outlier_scores);
	assert(singleton_report.notes.size() == 1);

	mtrc::stats::diagnose_options bad_neighbor;
	bad_neighbor.neighbor_count = 0;
	assert(throws<std::invalid_argument>([&] { (void)mtrc::stats::diagnose_space(space, bad_neighbor); }));

	mtrc::stats::diagnose_options bad_outlier;
	bad_outlier.outlier_neighbor_count = 0;
	assert(throws<std::invalid_argument>([&] { (void)mtrc::stats::diagnose_space(space, bad_outlier); }));

	return 0;
}
