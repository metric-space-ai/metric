#include <cassert>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"

struct LengthDelta {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> std::size_t
	{
		return lhs.size() > rhs.size() ? lhs.size() - rhs.size() : rhs.size() - lhs.size();
	}
};

struct ReorderedIdSource {
	auto id(std::size_t position) const -> mtrc::RecordId
	{
		const std::vector<mtrc::RecordId> ids{mtrc::RecordId::from_index(4), mtrc::RecordId::from_index(1),
												mtrc::RecordId::from_index(3)};
		return ids.at(position);
	}
};

struct LineDistanceMatrix {
	auto operator()(std::size_t row, std::size_t column) const -> int
	{
		return row > column ? static_cast<int>(row - column) : static_cast<int>(column - row);
	}
};

int main()
{
	std::vector<std::string> records = {"metric", "metrics", "matrix", "tree"};

	static_assert(mtrc::MetricCallable_v<mtrc::Edit<char>, std::string>);
	static_assert(mtrc::MetricCallable_v<LengthDelta, std::string>);

	mtrc::MetricSpace<std::string, mtrc::Edit<char>> typed_space(records, mtrc::Edit<char>{});
	static_assert(mtrc::MetricSpaceLike_v<decltype(typed_space)>);
	static_assert(mtrc::IndexedRecordIdSource_v<decltype(typed_space)>);

	const auto metric_id = typed_space.id(0);
	const auto metrics_id = typed_space.id(1);
	const auto matrix_id = typed_space.id(2);
	const auto tree_id = typed_space.id(3);
	static_assert(mtrc::IndexedRecordIdSource_v<ReorderedIdSource>);
	static_assert(std::is_same<decltype(typed_space.version()), mtrc::SpaceVersion>::value);
	assert(mtrc::initial_space_version == 0);
	assert(mtrc::next_space_version(mtrc::initial_space_version) == 1);
	assert(metric_id.index() == 0);
	assert(typed_space.size() == records.size());
	assert(!typed_space.empty());
	assert(typed_space.version() == 0);
	assert(typed_space.id_at(0) == metric_id);
	assert(typed_space.contains(metric_id));
	assert(typed_space.position_of(metrics_id) == 1);
	assert((mtrc::record_ids(typed_space) == std::vector<mtrc::RecordId>{metric_id, metrics_id, matrix_id, tree_id}));
	const auto initial_ids = mtrc::record_ids(typed_space);
	const auto sparse_position_ids = mtrc::record_ids_at_positions(typed_space, std::vector<std::size_t>{0, 3});
	assert((sparse_position_ids == std::vector<mtrc::RecordId>{metric_id, tree_id}));
	const auto sparse_records = mtrc::records_for_record_ids(typed_space, std::vector<mtrc::RecordId>{tree_id, metric_id});
	assert((sparse_records == std::vector<std::string>{"tree", "metric"}));
	auto length_transform = [](const std::string &record) -> std::size_t { return record.size(); };
	const auto transformed_lengths = mtrc::transformed_records(typed_space, length_transform);
	assert((transformed_lengths == std::vector<std::size_t>{6, 7, 6, 4}));
	assert(mtrc::contains_record_id(initial_ids, matrix_id));
	assert(!mtrc::contains_record_id(initial_ids, mtrc::RecordId::from_index(99)));
	assert(mtrc::position_of_record_id(initial_ids, tree_id, "missing test id") == 3);
	mtrc::require_record_ids_in_space(typed_space, std::vector<mtrc::RecordId>{metric_id, tree_id},
										"source id outside test space");
	bool rejected_source_id_outside_space = false;
	try {
		mtrc::require_record_ids_in_space(
			typed_space, std::vector<mtrc::RecordId>{mtrc::RecordId::from_index(99)},
			"source id outside test space");
	} catch (const std::invalid_argument &) {
		rejected_source_id_outside_space = true;
	}
	assert(rejected_source_id_outside_space);
	bool rejected_bad_record_position = false;
	try {
		(void)mtrc::record_ids_at_positions(typed_space, std::vector<std::size_t>{typed_space.size()});
	} catch (const std::out_of_range &) {
		rejected_bad_record_position = true;
	}
	assert(rejected_bad_record_position);
	bool rejected_bad_record_snapshot_id = false;
	try {
		(void)mtrc::records_for_record_ids(typed_space, std::vector<mtrc::RecordId>{mtrc::RecordId::from_index(99)});
	} catch (const std::out_of_range &) {
		rejected_bad_record_snapshot_id = true;
	}
	assert(rejected_bad_record_snapshot_id);
	assert(mtrc::record_id_overlap_count(initial_ids, std::vector<mtrc::RecordId>{tree_id, matrix_id}) == 2);
	assert(mtrc::record_id_overlap_count(initial_ids, std::vector<mtrc::RecordId>{metrics_id, metrics_id}) == 2);
	assert(mtrc::record_id_overlap_count(initial_ids, std::vector<mtrc::RecordId>{mtrc::RecordId::from_index(99)}) ==
		   0);
	mtrc::RecallAccumulator recall;
	assert(recall.total_recall(1.0) == 1.0);
	assert(recall.average_recall(1.0) == 1.0);
	assert(recall.minimum_recall_or(1.0) == 1.0);
	assert(recall.maximum_recall_or(1.0) == 1.0);
	assert(recall.add(1, 2) == 0.5);
	assert(recall.add(0, 0, 1.0) == 1.0);
	assert(recall.matched == 1);
	assert(recall.possible == 2);
	assert(recall.observations == 2);
	assert(recall.total_recall() == 0.5);
	assert(recall.average_recall() == 0.75);
	assert(recall.minimum_recall_or() == 0.5);
	assert(recall.maximum_recall_or() == 1.0);
	mtrc::ScalarAccumulator<int> scalar;
	assert(scalar.average_or(9.0) == 9.0);
	assert(scalar.minimum_or(4) == 4);
	assert(scalar.maximum_or(4) == 4);
	scalar.add(4);
	scalar.add(1);
	scalar.add(7);
	assert(scalar.observations == 3);
	assert(scalar.sum == 12.0);
	assert(scalar.average_or() == 4.0);
	assert(scalar.minimum_or() == 1);
	assert(scalar.maximum_or() == 7);
	const auto scalar_summary = mtrc::summarize_scalars(std::vector<int>{4, 1, 7});
	assert(scalar_summary.observations == 3);
	assert(scalar_summary.sum == 12.0);
	assert(scalar_summary.average_or() == 4.0);
	assert(scalar_summary.minimum_or() == 1);
	assert(scalar_summary.maximum_or() == 7);
	const auto empty_scalar_summary = mtrc::summarize_scalars(std::vector<int>{});
	assert(empty_scalar_summary.observations == 0);
	assert(empty_scalar_summary.average_or(9.0) == 9.0);
	assert(empty_scalar_summary.minimum_or(4) == 4);
	assert(empty_scalar_summary.maximum_or(4) == 4);
	const auto line_expansion_dimension = mtrc::core::expansion_dimension(LineDistanceMatrix{}, 5, 0);
	assert(std::abs(line_expansion_dimension - (std::log(5.0 / 3.0) / std::log(2.0))) < 1e-12);
	assert(mtrc::core::expansion_dimension(LineDistanceMatrix{}, 0, 0) == 0.0);
	assert(mtrc::core::expansion_dimension(LineDistanceMatrix{}, 1, 0) == 0.0);
	const auto assignment_ids = mtrc::core::record_ids_excluding_assignment(
		typed_space, std::vector<std::size_t>{0, mtrc::ClusteringResult<std::size_t>::noise_label, 1, 1},
		mtrc::ClusteringResult<std::size_t>::noise_label);
	assert((assignment_ids == std::vector<mtrc::RecordId>{metric_id, matrix_id, tree_id}));
	const auto matching_assignment_ids = mtrc::core::record_ids_matching_value(
		typed_space, std::vector<std::size_t>{0, mtrc::ClusteringResult<std::size_t>::noise_label, 1,
											  mtrc::ClusteringResult<std::size_t>::noise_label},
		mtrc::ClusteringResult<std::size_t>::noise_label);
	assert((matching_assignment_ids == std::vector<mtrc::RecordId>{metrics_id, tree_id}));
	const auto matching_flag_ids =
		mtrc::core::record_ids_matching_value(typed_space, std::vector<bool>{true, false, true, false}, true);
	assert((matching_flag_ids == std::vector<mtrc::RecordId>{metric_id, matrix_id}));
	mtrc::ClusteringResult<std::size_t> grouped_records;
	grouped_records.assignments = {0, mtrc::ClusteringResult<std::size_t>::noise_label, 1,
								   mtrc::ClusteringResult<std::size_t>::noise_label};
	grouped_records.record_count = typed_space.size();
	mtrc::core::require_clustering_result_shape(grouped_records, typed_space.size(), "bad clustering record count",
												  "bad clustering assignment count");
	bool rejected_bad_clustering_record_count = false;
	try {
		mtrc::core::require_clustering_result_shape(grouped_records, typed_space.size() + 1,
													  "bad clustering record count", "bad clustering assignment count");
	} catch (const std::invalid_argument &) {
		rejected_bad_clustering_record_count = true;
	}
	assert(rejected_bad_clustering_record_count);
	auto bad_assignment_count = grouped_records;
	bad_assignment_count.assignments.pop_back();
	bool rejected_bad_clustering_assignment_count = false;
	try {
		mtrc::core::require_clustering_result_shape(bad_assignment_count, typed_space.size(),
													  "bad clustering record count", "bad clustering assignment count");
	} catch (const std::invalid_argument &) {
		rejected_bad_clustering_assignment_count = true;
	}
	assert(rejected_bad_clustering_assignment_count);
	const auto grouped_noise_ids = mtrc::core::noise_record_ids(typed_space, grouped_records);
	assert((grouped_noise_ids == std::vector<mtrc::RecordId>{metrics_id, tree_id}));
	const auto grouped_non_noise_ids = mtrc::core::non_noise_record_ids(typed_space, grouped_records);
	assert((grouped_non_noise_ids == std::vector<mtrc::RecordId>{metric_id, matrix_id}));
	grouped_records.cluster_count = 2;
	const auto direct_assignment_buckets = mtrc::core::record_id_buckets_for_assignments(
		typed_space, std::vector<std::size_t>{0, 1, 1, 0}, grouped_records.cluster_count);
	assert(direct_assignment_buckets.size() == 2);
	assert((direct_assignment_buckets[0] == std::vector<mtrc::RecordId>{metric_id, tree_id}));
	assert((direct_assignment_buckets[1] == std::vector<mtrc::RecordId>{metrics_id, matrix_id}));
	bool rejected_bad_direct_assignment_bucket = false;
	try {
		(void)mtrc::core::record_id_buckets_for_assignments(
			typed_space, std::vector<std::size_t>{0, 2}, 2, "assignment label outside buckets");
	} catch (const std::invalid_argument &) {
		rejected_bad_direct_assignment_bucket = true;
	}
	assert(rejected_bad_direct_assignment_bucket);
	const auto assignment_buckets = mtrc::core::record_id_buckets_excluding_assignment(
		typed_space, grouped_records.assignments, grouped_records.cluster_count,
		mtrc::ClusteringResult<std::size_t>::noise_label);
	assert(assignment_buckets.size() == 2);
	assert((assignment_buckets[0] == std::vector<mtrc::RecordId>{metric_id}));
	assert((assignment_buckets[1] == std::vector<mtrc::RecordId>{matrix_id}));
	const auto grouped_member_ids = mtrc::core::cluster_member_record_ids(typed_space, grouped_records);
	assert(grouped_member_ids == assignment_buckets);
	bool rejected_bad_assignment_bucket = false;
	try {
		(void)mtrc::core::record_id_buckets_excluding_assignment(
			typed_space, std::vector<std::size_t>{0, 2}, 2,
			mtrc::ClusteringResult<std::size_t>::noise_label, "assignment label outside buckets");
	} catch (const std::invalid_argument &) {
		rejected_bad_assignment_bucket = true;
	}
	assert(rejected_bad_assignment_bucket);
	const auto scored_outliers = mtrc::core::scored_outliers<int>(
		std::vector<mtrc::RecordId>{tree_id, metric_id, matrix_id}, [metric_id, matrix_id](mtrc::RecordId id) {
			if (id == metric_id || id == matrix_id) {
				return 7;
			}
			return 3;
		});
	assert(scored_outliers.size() == 3);
	assert(scored_outliers[0].id == metric_id);
	assert(scored_outliers[0].score == 7);
	assert(scored_outliers[1].id == matrix_id);
	assert(scored_outliers[1].score == 7);
	assert(scored_outliers[2].id == tree_id);
	assert(scored_outliers[2].score == 3);
	std::vector<mtrc::Outlier<int>> manual_outliers{{tree_id, 2}, {matrix_id, 5}, {metrics_id, 5}};
	mtrc::core::sort_outliers(manual_outliers);
	assert(manual_outliers[0].id == metrics_id);
	assert(manual_outliers[1].id == matrix_id);
	assert(manual_outliers[2].id == tree_id);
	const auto empty_outliers = mtrc::core::scored_outliers<int>(std::vector<mtrc::RecordId>{},
																   [](mtrc::RecordId) { return 1; });
	assert(empty_outliers.empty());
	auto lineage = mtrc::one_to_one_lineage(typed_space);
	assert((lineage.representative_records == std::vector<mtrc::RecordId>{metric_id, metrics_id, matrix_id, tree_id}));
	assert(lineage.source_records.size() == typed_space.size());
	assert((lineage.source_records[0] == std::vector<mtrc::RecordId>{metric_id}));
	assert((lineage.source_records[1] == std::vector<mtrc::RecordId>{metrics_id}));
	auto subset_lineage = mtrc::one_to_one_lineage(std::vector<mtrc::RecordId>{tree_id, metric_id});
	assert((subset_lineage.representative_records == std::vector<mtrc::RecordId>{tree_id, metric_id}));
	assert((subset_lineage.source_records[0] == std::vector<mtrc::RecordId>{tree_id}));
	assert((subset_lineage.source_records[1] == std::vector<mtrc::RecordId>{metric_id}));
	const auto extracted_source_ids = mtrc::one_to_one_lineage_source_ids(
		lineage.source_records, typed_space.size(), "bad row count", "bad row shape", "duplicate source id");
	assert((extracted_source_ids == std::vector<mtrc::RecordId>{metric_id, metrics_id, matrix_id, tree_id}));
	const auto extracted_source_ids_in_space = mtrc::one_to_one_lineage_source_ids_in_space(
		typed_space, lineage.source_records, typed_space.size(), "bad row count", "bad row shape",
		"duplicate source id", "source id outside test space");
	assert((extracted_source_ids_in_space ==
			std::vector<mtrc::RecordId>{metric_id, metrics_id, matrix_id, tree_id}));
	bool rejected_lineage_source_outside_space = false;
	try {
		(void)mtrc::one_to_one_lineage_source_ids_in_space(
			typed_space, std::vector<std::vector<mtrc::RecordId>>{{mtrc::RecordId::from_index(99)}}, 1,
			"bad row count", "bad row shape", "duplicate source id", "source id outside test space");
	} catch (const std::invalid_argument &) {
		rejected_lineage_source_outside_space = true;
	}
	assert(rejected_lineage_source_outside_space);
	bool rejected_bad_lineage_row_count = false;
	try {
		(void)mtrc::one_to_one_lineage_source_ids(lineage.source_records, typed_space.size() + 1, "bad row count",
													"bad row shape", "duplicate source id");
	} catch (const std::invalid_argument &) {
		rejected_bad_lineage_row_count = true;
	}
	assert(rejected_bad_lineage_row_count);
	bool rejected_bad_lineage_shape = false;
	try {
		(void)mtrc::one_to_one_lineage_source_ids(std::vector<std::vector<mtrc::RecordId>>{{metric_id, tree_id}},
													1, "bad row count", "bad row shape", "duplicate source id");
	} catch (const std::invalid_argument &) {
		rejected_bad_lineage_shape = true;
	}
	assert(rejected_bad_lineage_shape);
	bool rejected_duplicate_lineage_source = false;
	try {
		(void)mtrc::one_to_one_lineage_source_ids(
			std::vector<std::vector<mtrc::RecordId>>{{metric_id}, {metric_id}}, 2, "bad row count",
			"bad row shape", "duplicate source id");
	} catch (const std::invalid_argument &) {
		rejected_duplicate_lineage_source = true;
	}
	assert(rejected_duplicate_lineage_source);
	bool rejected_missing_helper_id = false;
	try {
		(void)mtrc::position_of_record_id(initial_ids, mtrc::RecordId::from_index(99), "missing test id");
	} catch (const std::out_of_range &) {
		rejected_missing_helper_id = true;
	}
	assert(rejected_missing_helper_id);
	assert(typed_space[metric_id] == "metric");
	assert(typed_space.distance(metric_id, metrics_id) == 1);
	assert(typed_space(metric_id, metrics_id) == 1);

	const auto inserted_id = typed_space.insert("metrician");
	assert(typed_space.version() == 1);
	assert(typed_space.size() == records.size() + 1);
	assert(typed_space.contains(inserted_id));
	assert(typed_space.position_of(inserted_id) == records.size());
	assert(typed_space.record(inserted_id) == "metrician");

	typed_space.replace(inserted_id, "forest");
	assert(typed_space.version() == 2);
	assert(typed_space.record(inserted_id) == "forest");

	assert(typed_space.erase(metrics_id));
	assert(typed_space.version() == 3);
	assert(!typed_space.contains(metrics_id));
	assert(typed_space.position_of(inserted_id) == records.size() - 1);
	assert(typed_space.id_at(0) == metric_id);
	assert((mtrc::record_ids(typed_space) == std::vector<mtrc::RecordId>{metric_id, matrix_id, tree_id, inserted_id}));
	lineage = mtrc::one_to_one_lineage(typed_space);
	assert((lineage.representative_records == std::vector<mtrc::RecordId>{metric_id, matrix_id, tree_id, inserted_id}));
	assert(lineage.source_records.size() == typed_space.size());
	assert((lineage.source_records[3] == std::vector<mtrc::RecordId>{inserted_id}));
	bool rejected_erased_id = false;
	try {
		(void)typed_space.position_of(metrics_id);
	} catch (const std::out_of_range &) {
		rejected_erased_id = true;
	}
	assert(rejected_erased_id);
	assert(!typed_space.erase(metrics_id));

	const auto made_space = mtrc::make_space(records, mtrc::Edit<char>{});
	static_assert(mtrc::MetricSpaceLike_v<decltype(made_space)>);
	assert(made_space.distance(made_space.id(0), made_space.id(1)) == 1);
	assert(made_space.version() == 0);

	const auto custom_space = mtrc::make_space(records, LengthDelta{});
	assert(custom_space.distance(custom_space.id(0), custom_space.id(3)) == 2);
	assert(mtrc::metric_traits<LengthDelta>::law == mtrc::metric_law::distance);
	assert(mtrc::metric_traits<LengthDelta>::records == mtrc::record_kind::custom);
	const auto custom_provider = mtrc::space::storage::implicit(custom_space);
	static_assert(mtrc::PairwiseDistances_v<decltype(custom_provider)>);
	static_assert(mtrc::IndexedRecordIdSource_v<decltype(custom_provider)>);
	const auto provider_position_ids =
		mtrc::record_ids_at_positions(custom_provider, std::vector<std::size_t>{3, 0});
	assert((provider_position_ids == std::vector<mtrc::RecordId>{custom_space.id(3), custom_space.id(0)}));
	bool rejected_bad_provider_record_position = false;
	try {
		(void)mtrc::record_ids_at_positions(custom_provider, std::vector<std::size_t>{custom_provider.record_count()});
	} catch (const std::out_of_range &) {
		rejected_bad_provider_record_position = true;
	}
	assert(rejected_bad_provider_record_position);
	assert(mtrc::total_distance_to_provider_records(custom_provider, custom_space.id(0)) == 3);
	assert(mtrc::total_distance_to_record_ids(custom_provider, custom_space.id(0),
												std::vector<mtrc::RecordId>{custom_space.id(0), custom_space.id(3)}) ==
		   2);
	assert(mtrc::total_distance_to_record_ids(custom_provider, custom_space.id(0), std::vector<mtrc::RecordId>{}) ==
		   0);
	assert(mtrc::minimum_total_distance_record_id(
			   custom_provider, std::vector<mtrc::RecordId>{custom_space.id(0), custom_space.id(3)},
			   "candidate ids must not be empty", "candidate id is outside provider") == custom_space.id(0));
	assert(mtrc::minimum_total_distance_record_id(
			   custom_provider, std::vector<mtrc::RecordId>{custom_space.id(1), custom_space.id(2), custom_space.id(3)},
			   "candidate ids must not be empty", "candidate id is outside provider") == custom_space.id(2));
	const auto provider_distance_table = mtrc::distance_table_for_record_ids(
		custom_provider, std::vector<mtrc::RecordId>{custom_space.id(0), custom_space.id(3)},
		"candidate id is outside provider");
	assert(provider_distance_table.size() == 2);
	assert(provider_distance_table[0].size() == 2);
	assert(provider_distance_table[0][0] == 0);
	assert(provider_distance_table[0][1] == 2);
	assert(provider_distance_table[1][0] == 2);
	assert(provider_distance_table[1][1] == 0);
	assert(mtrc::distance_table_for_record_ids(custom_provider, std::vector<mtrc::RecordId>{},
												 "candidate id is outside provider")
			   .empty());
	auto distances_to_metric =
		mtrc::distances_to_record_id(custom_provider, custom_space.id(0), "target id is outside provider");
	assert((distances_to_metric == std::vector<std::size_t>{0, 1, 0, 2}));
	assert(mtrc::farthest_unselected_position(
			   distances_to_metric, std::vector<bool>{true, false, true, false},
			   "selection count does not match distances", "no unselected distance") == 3);
	assert(mtrc::farthest_unselected_position(
			   std::vector<std::size_t>{0, 4, 4, 2}, std::vector<bool>{true, false, false, false},
			   "selection count does not match distances", "no unselected distance") == 1);
	assert(mtrc::farthest_unselected_record_position(
			   ReorderedIdSource{}, std::vector<std::size_t>{4, 4, 2}, std::vector<bool>{false, false, false},
			   "selection count does not match distances", "no unselected distance") == 1);
	assert(mtrc::farthest_unselected_record_position(
			   ReorderedIdSource{}, std::vector<std::size_t>{4, 4, 2}, std::vector<bool>{false, true, false},
			   "selection count does not match distances", "no unselected distance") == 0);
	assert(mtrc::first_unmarked_position(std::vector<bool>{true, false, true}, "no unmarked record") == 1);
	std::vector<bool> covered_records{false, false, true, false};
	const auto newly_covered = mtrc::mark_records_within_radius(
		custom_provider, custom_space.id(0), std::size_t{1}, covered_records,
		"coverage state count does not match provider", "coverage seed id is outside provider");
	assert(newly_covered == 2);
	assert((covered_records == std::vector<bool>{true, true, true, false}));
	mtrc::update_min_distances_to_record_id(
		custom_provider, distances_to_metric, custom_space.id(3),
		"distance vector count does not match provider", "target id is outside provider");
	assert((distances_to_metric == std::vector<std::size_t>{0, 1, 0, 0}));
	assert(mtrc::nearest_distance_to_record_ids(
			   custom_provider, custom_space.id(3), std::vector<mtrc::RecordId>{custom_space.id(0), custom_space.id(1)},
			   "reference ids must not be empty") == 2);
	assert(mtrc::record_is_separated_from_record_ids(
		custom_provider, custom_space.id(3), std::vector<mtrc::RecordId>{custom_space.id(0)}, std::size_t{2},
		"candidate id is outside provider", "reference id is outside provider"));
	assert(!mtrc::record_is_separated_from_record_ids(
		custom_provider, custom_space.id(1), std::vector<mtrc::RecordId>{custom_space.id(0)}, std::size_t{2},
		"candidate id is outside provider", "reference id is outside provider"));
	assert(mtrc::record_is_separated_from_record_ids(custom_provider, custom_space.id(1),
													   std::vector<mtrc::RecordId>{}, std::size_t{2},
													   "candidate id is outside provider",
													   "reference id is outside provider"));
	assert(mtrc::nearest_other_record_distance_or(custom_provider, custom_space.id(3), 99) == 2);
	const auto representative_assignment = mtrc::assign_records_to_representatives(
		custom_provider, std::vector<mtrc::RecordId>{custom_space.id(0), custom_space.id(3)},
		"representative set must not be empty", "representative id is outside provider");
	assert((representative_assignment.assignments == std::vector<std::size_t>{0, 0, 0, 1}));
	assert((representative_assignment.nearest_distances == std::vector<std::size_t>{0, 1, 0, 0}));
	const auto tie_assignment = mtrc::assign_records_to_representatives(
		custom_provider, std::vector<mtrc::RecordId>{custom_space.id(2), custom_space.id(0)},
		"representative set must not be empty", "representative id is outside provider");
	assert((tie_assignment.assignments == std::vector<std::size_t>{1, 1, 1, 1}));
	bool rejected_empty_representatives = false;
	try {
		(void)mtrc::assign_records_to_representatives(custom_provider, std::vector<mtrc::RecordId>{},
														"representative set must not be empty",
														"representative id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_empty_representatives = true;
	}
	assert(rejected_empty_representatives);
	bool rejected_missing_representative = false;
	try {
		(void)mtrc::assign_records_to_representatives(
			custom_provider, std::vector<mtrc::RecordId>{mtrc::RecordId::from_index(99)},
			"representative set must not be empty", "representative id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_missing_representative = true;
	}
	assert(rejected_missing_representative);
	bool rejected_empty_reference_ids = false;
	try {
		(void)mtrc::nearest_distance_to_record_ids(custom_provider, custom_space.id(0),
													 std::vector<mtrc::RecordId>{}, "reference ids must not be empty");
	} catch (const std::invalid_argument &) {
		rejected_empty_reference_ids = true;
	}
	assert(rejected_empty_reference_ids);
	bool rejected_empty_minimum_total_candidates = false;
	try {
		(void)mtrc::minimum_total_distance_record_id(custom_provider, std::vector<mtrc::RecordId>{},
													   "candidate ids must not be empty",
													   "candidate id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_empty_minimum_total_candidates = true;
	}
	assert(rejected_empty_minimum_total_candidates);
	bool rejected_missing_minimum_total_candidate = false;
	try {
		(void)mtrc::minimum_total_distance_record_id(
			custom_provider, std::vector<mtrc::RecordId>{mtrc::RecordId::from_index(99)},
			"candidate ids must not be empty", "candidate id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_missing_minimum_total_candidate = true;
	}
	assert(rejected_missing_minimum_total_candidate);
	bool rejected_missing_distance_table_id = false;
	try {
		(void)mtrc::distance_table_for_record_ids(
			custom_provider, std::vector<mtrc::RecordId>{mtrc::RecordId::from_index(99)},
			"candidate id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_missing_distance_table_id = true;
	}
	assert(rejected_missing_distance_table_id);
	bool rejected_missing_distance_target_id = false;
	try {
		(void)mtrc::distances_to_record_id(custom_provider, mtrc::RecordId::from_index(99),
											 "target id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_missing_distance_target_id = true;
	}
	assert(rejected_missing_distance_target_id);
	bool rejected_bad_distance_update_size = false;
	try {
		auto bad_distances = std::vector<std::size_t>{0};
		mtrc::update_min_distances_to_record_id(custom_provider, bad_distances, custom_space.id(0),
												  "distance vector count does not match provider",
												  "target id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_bad_distance_update_size = true;
	}
	assert(rejected_bad_distance_update_size);
	bool rejected_bad_farthest_selection_size = false;
	try {
		(void)mtrc::farthest_unselected_position(
			std::vector<std::size_t>{0}, std::vector<bool>{true, false},
			"selection count does not match distances", "no unselected distance");
	} catch (const std::invalid_argument &) {
		rejected_bad_farthest_selection_size = true;
	}
	assert(rejected_bad_farthest_selection_size);
	bool rejected_exhausted_farthest_selection = false;
	try {
		(void)mtrc::farthest_unselected_position(
			std::vector<std::size_t>{0, 1}, std::vector<bool>{true, true},
			"selection count does not match distances", "no unselected distance");
	} catch (const std::logic_error &) {
		rejected_exhausted_farthest_selection = true;
	}
	assert(rejected_exhausted_farthest_selection);
	bool rejected_bad_farthest_record_selection_size = false;
	try {
		(void)mtrc::farthest_unselected_record_position(
			ReorderedIdSource{}, std::vector<std::size_t>{0}, std::vector<bool>{true, false},
			"selection count does not match distances", "no unselected distance");
	} catch (const std::invalid_argument &) {
		rejected_bad_farthest_record_selection_size = true;
	}
	assert(rejected_bad_farthest_record_selection_size);
	bool rejected_exhausted_farthest_record_selection = false;
	try {
		(void)mtrc::farthest_unselected_record_position(
			ReorderedIdSource{}, std::vector<std::size_t>{0, 1}, std::vector<bool>{true, true},
			"selection count does not match distances", "no unselected distance");
	} catch (const std::logic_error &) {
		rejected_exhausted_farthest_record_selection = true;
	}
	assert(rejected_exhausted_farthest_record_selection);
	bool rejected_exhausted_unmarked_position = false;
	try {
		(void)mtrc::first_unmarked_position(std::vector<bool>{true, true}, "no unmarked record");
	} catch (const std::logic_error &) {
		rejected_exhausted_unmarked_position = true;
	}
	assert(rejected_exhausted_unmarked_position);
	bool rejected_bad_coverage_mark_size = false;
	try {
		auto bad_covered = std::vector<bool>{false};
		(void)mtrc::mark_records_within_radius(custom_provider, custom_space.id(0), std::size_t{1}, bad_covered,
												 "coverage state count does not match provider",
												 "coverage seed id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_bad_coverage_mark_size = true;
	}
	assert(rejected_bad_coverage_mark_size);
	bool rejected_missing_coverage_seed = false;
	try {
		auto valid_covered = std::vector<bool>(custom_provider.record_count(), false);
		(void)mtrc::mark_records_within_radius(custom_provider, mtrc::RecordId::from_index(99), std::size_t{1},
												 valid_covered, "coverage state count does not match provider",
												 "coverage seed id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_missing_coverage_seed = true;
	}
	assert(rejected_missing_coverage_seed);
	bool rejected_missing_separation_candidate = false;
	try {
		(void)mtrc::record_is_separated_from_record_ids(
			custom_provider, mtrc::RecordId::from_index(99), std::vector<mtrc::RecordId>{custom_space.id(0)},
			std::size_t{1}, "candidate id is outside provider", "reference id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_missing_separation_candidate = true;
	}
	assert(rejected_missing_separation_candidate);
	bool rejected_missing_separation_reference = false;
	try {
		(void)mtrc::record_is_separated_from_record_ids(
			custom_provider, custom_space.id(0), std::vector<mtrc::RecordId>{mtrc::RecordId::from_index(99)},
			std::size_t{1}, "candidate id is outside provider", "reference id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_missing_separation_reference = true;
	}
	assert(rejected_missing_separation_reference);
	bool rejected_missing_distance_update_target = false;
	try {
		auto valid_distances = std::vector<std::size_t>{0, 1, 0, 2};
		mtrc::update_min_distances_to_record_id(custom_provider, valid_distances, mtrc::RecordId::from_index(99),
												  "distance vector count does not match provider",
												  "target id is outside provider");
	} catch (const std::invalid_argument &) {
		rejected_missing_distance_update_target = true;
	}
	assert(rejected_missing_distance_update_target);
	const auto singleton_space = mtrc::make_space(std::vector<std::string>{"solo"}, LengthDelta{});
	const auto singleton_provider = mtrc::space::storage::implicit(singleton_space);
	assert(mtrc::nearest_other_record_distance_or(singleton_provider, singleton_space.id(0), 99) == 99);

	bool rejected_bad_id = false;
	try {
		(void)custom_space.id(records.size());
	} catch (const std::out_of_range &) {
		rejected_bad_id = true;
	}
	assert(rejected_bad_id);

	return 0;
}
