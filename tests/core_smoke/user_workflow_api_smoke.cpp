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

	static_assert(mtrc::metric::is_discoverable_metric_v<metric_type>,
				  "Edit must be discoverable as a true metric");
	static_assert(mtrc::metric::admission_is_true_metric(mtrc::metric::admission_status_v<metric_type>),
				  "Edit admission status must be metric-compatible");

	const std::vector<record_type> input = {"pump_ok", "pump_warn", "pump_fail", "valve_ok"};
	auto records = mtrc::record::import_records(input);
	mtrc::record::validate_records_non_empty(records);

	auto direct_space = mtrc::space::build_checked(records, metric_type{});
	assert(direct_space.size() == records.size());

	auto described = mtrc::space::space_builder<record_type>(metric_type{})
						 .add_all(records)
						 .require_non_empty()
						 .require_true_metric()
						 .named("maintenance-events")
						 .build_described();
	assert(described.metadata.contains("name"));

	auto space = std::move(described.space);
	assert(space.size() == 4);

	const auto ids = mtrc::space::records::ids(space);
	assert(ids.size() == 4);
	assert(mtrc::space::records::contains_all(space, ids));

	auto table = mtrc::space::distances::materialize(space);
	auto table_status = mtrc::space::distances::status(table);
	assert(table_status.materialized);
	assert(table_status.exact);
	assert(!table_status.stale);

	const auto pair_value = mtrc::space::distances::value(space, ids[0], ids[1]);
	assert(pair_value == mtrc::space::distances::checked_value(table, ids[0], ids[1]));
	const auto row = mtrc::space::distances::row(space, ids[0]);
	assert(row.size() == space.size());
	const auto pairs = mtrc::space::distances::pairs(space);
	assert(pairs.size() == 6);

	const auto nearest = mtrc::space::query::nearest(space, ids[0]);
	assert(nearest.id != ids[0]);
	const auto knn = mtrc::space::query::k_nearest(space, ids[0], 2);
	assert(knn.size() == 2);
	assert(knn.operator_name == "knn");
	const auto within = mtrc::space::query::within(space, ids[0], 4);
	assert(within.operator_name == "range");

	const auto batch = mtrc::stats::search::knn_batch(space, std::vector<mtrc::RecordId>{ids[0], ids[1]}, 1);
	assert(batch.size() == 2);
	assert(batch[0].size() == 1);

	auto profile_options = mtrc::stats::properties::profile_options{};
	profile_options.include_distance_distribution = true;
	profile_options.distribution.bucket_count = 4;
	profile_options.include_local_volume = true;
	profile_options.local_volume_radius = 4.0;
	const auto profile = mtrc::stats::profile(space, profile_options);
	assert(profile.record_count == space.size());
	assert(profile.pair_count == 6);
	assert(profile.has_distance_distribution);
	assert(profile.distance_distribution.bucket_count() == 4);
	assert(profile.has_local_volume);
	assert(profile.local_volume.record_count == space.size());

	const auto regular_sample = mtrc::stats::sample::regular_sample(space, 2);
	assert(regular_sample.size() == 2);
	assert(regular_sample.algorithm == "regular_sample");

	const auto representatives = mtrc::stats::structural_analysis::representatives(records, metric_type{}, 2);
	assert(representatives.size() == 2);

	const auto outliers =
		mtrc::stats::structural_analysis::find_outliers(space, mtrc::stats::structural_analysis::dbscan_options(3.0, 2));
	assert(outliers.record_count == space.size());
	assert(outliers.operator_name == "find_outliers");

	const auto represented = mtrc::modify::represent::represent(space, 2);
	assert(represented.size() == 2);
	assert(represented.source_record_count == space.size());
	assert(represented.metric_status == mtrc::metric_law::metric);

	const auto compressed = mtrc::modify::reduce::compress(space, 2);
	assert(compressed.size() == 2);
	assert(compressed.source_record_count == space.size());
	assert(compressed.metric_status == mtrc::metric_law::metric);

	const auto subspace = mtrc::space::select_subspace(space, std::vector<mtrc::RecordId>{ids[0], ids[1]});
	assert(mtrc::space::parent_record_id(subspace, subspace.space.id(0)) == ids[0]);

	const auto mutation = mtrc::space::records::insert(space, std::vector<record_type>{"pump_fixed"});
	assert(mutation.changed());
	assert(mutation.count() == 1);
	assert(space.size() == 5);
	assert(mtrc::space::cache::is_stale(table));
	assert(mtrc::space::distances::status(table).stale);
	assert(throws<mtrc::StaleRepresentationError>(
		[&] { (void)mtrc::space::distances::checked_value(table, ids[0], ids[1]); }));

	auto rebuilt_table = mtrc::space::cache::rebuild(table, space);
	assert(!mtrc::space::cache::is_stale(rebuilt_table));
	assert(mtrc::space::distances::checked_value(rebuilt_table, ids[0], ids[1]) == pair_value);

	const auto edits = std::vector<mtrc::space::records::edit<record_type>>{{ids[1], "pump_warning"}};
	const auto edit_report = mtrc::space::records::replace(space, edits);
	assert(edit_report.changed());
	assert(edit_report.affected_ids.size() == 1);

	assert(throws<mtrc::MetricInputError>([&] {
		(void)mtrc::space::space_builder<record_type>(metric_type{}).require_non_empty().build();
	}));

	return 0;
}
