#include <cassert>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/workflow.hpp>

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
	using metric_type = mtrc::Edit<char>;

	auto space = mtrc::space::build_checked(
		std::vector<std::string>{"pump ok\nline", "pump warn", "pump fail"}, metric_type{});
	const auto ok = space.id(0);
	const auto warn = space.id(1);
	const auto fail = space.id(2);

	assert(space.erase(warn));
	const auto fixed = space.insert("pump fixed");
	assert(fixed.index() == 3);
	assert(space.next_record_id() == 4);
	assert(space.version() == 2);

	const auto artifact = mtrc::space::persistence::export_space(space);
	assert(artifact.record_count() == 3);
	assert(artifact.pair_count() == 3);
	assert(artifact.next_record_id == 4);
	assert(artifact.space_version == space.version());
	assert(artifact.records[0].id == ok);
	assert(artifact.records[1].id == fail);
	assert(artifact.records[2].id == fixed);
	assert(artifact.records[0].record == "pump ok\nline");
	assert(artifact.metric_law == "metric");

	const auto verification = mtrc::space::persistence::verify_distances(space, artifact);
	assert(verification.ok());
	assert(verification.checked == 3);

	auto limited_options = mtrc::space::persistence::artifact_options{};
	limited_options.max_materialized_pair_distances = 2;
	assert(throws<mtrc::MetricInputError>(
		[&] { (void)mtrc::space::persistence::export_space(space, limited_options); }));

	limited_options.include_pair_distances = false;
	const auto metadata_only_artifact = mtrc::space::persistence::export_space(space, limited_options);
	assert(!metadata_only_artifact.pair_distances_included);
	assert(metadata_only_artifact.pair_count() == 0);

	limited_options.include_pair_distances = true;
	limited_options.allow_unbounded_pair_distances = true;
	const auto explicit_pair_artifact = mtrc::space::persistence::export_space(space, limited_options);
	assert(explicit_pair_artifact.pair_distances_included);
	assert(explicit_pair_artifact.pair_count() == artifact.pair_count());

	auto restored = mtrc::space::persistence::restore_space(artifact, metric_type{});
	assert(restored.size() == space.size());
	assert(restored.version() == space.version());
	assert(restored.next_record_id() == 4);
	assert(restored.id(0) == ok);
	assert(restored.id(1) == fail);
	assert(restored.id(2) == fixed);
	assert(!restored.contains(warn));
	assert(restored.record(ok) == "pump ok\nline");
	assert(restored.distance(ok, fixed) == space.distance(ok, fixed));
	assert(restored.insert("pump replaced").index() == 4);

	std::stringstream stream;
	mtrc::space::persistence::save(stream, space);
	const auto loaded = mtrc::space::persistence::load<std::string>(stream, metric_type{});
	assert(loaded.artifact.record_count() == artifact.record_count());
	assert(loaded.space.version() == space.version());
	assert(loaded.space.next_record_id() == 4);
	assert(loaded.space.record(ok) == "pump ok\nline");
	assert(loaded.space.distance(ok, fail) == space.distance(ok, fail));

	auto bad_key = artifact;
	bad_key.metric_key = "wrong.metric.key";
	assert(throws<mtrc::MetricInputError>([&] { (void)mtrc::space::persistence::restore_space(bad_key, metric_type{}); }));

	auto bad_distance = artifact;
	bad_distance.distances.front().distance += 1;
	const auto bad_verification = mtrc::space::persistence::verify_distances(space, bad_distance);
	assert(!bad_verification.ok());
	assert(bad_verification.mismatches.size() == 1);
	assert(throws<mtrc::MetricInputError>(
		[&] { (void)mtrc::space::persistence::restore_space(bad_distance, metric_type{}); }));

	auto subspace = mtrc::space::select_subspace(space, std::vector<mtrc::RecordId>{ok, fixed});
	const auto sub_artifact = mtrc::space::persistence::export_subspace(subspace);
	assert(sub_artifact.record_count() == 2);
	assert(sub_artifact.lineage.size() == 2);
	assert(sub_artifact.lineage[0].local_id == subspace.space.id(0));
	assert(sub_artifact.lineage[0].parent_id == ok);
	assert(sub_artifact.lineage[1].parent_id == fixed);

	auto restored_subspace = mtrc::space::persistence::restore_subspace(sub_artifact, metric_type{});
	assert(restored_subspace.space.size() == subspace.space.size());
	assert(mtrc::space::parent_record_id(restored_subspace, restored_subspace.space.id(0)) == ok);
	assert(mtrc::space::parent_record_id(restored_subspace, restored_subspace.space.id(1)) == fixed);

	std::stringstream sub_stream;
	mtrc::space::persistence::save(sub_stream, subspace);
	const auto loaded_subspace = mtrc::space::persistence::load_subspace<std::string>(sub_stream, metric_type{});
	assert(mtrc::space::parent_record_id(loaded_subspace.subspace, loaded_subspace.subspace.space.id(1)) == fixed);

	auto stale_subspace = subspace;
	stale_subspace.space.insert("new local record");
	assert(throws<mtrc::StaleRepresentationError>(
		[&] { (void)mtrc::space::persistence::export_subspace(stale_subspace); }));

	auto bad_sub_artifact = sub_artifact;
	bad_sub_artifact.lineage.pop_back();
	assert(throws<mtrc::MetricInputError>(
		[&] { (void)mtrc::space::persistence::restore_subspace(bad_sub_artifact, metric_type{}); }));

	auto left = mtrc::space::select_subspace(space, std::vector<mtrc::RecordId>{ok});
	auto right = mtrc::space::select_subspace(space, std::vector<mtrc::RecordId>{fail, fixed});
	auto merged = mtrc::space::merge_checked(
		std::vector<const decltype(left.space) *>{&left.space, &right.space});
	const auto merged_artifact = mtrc::space::persistence::export_merged_space(merged);
	assert(merged_artifact.record_count() == 3);
	assert(merged_artifact.lineage.size() == 3);
	assert(merged_artifact.lineage[0].source_index == 0);
	assert(merged_artifact.lineage[0].source_id == left.space.id(0));
	assert(merged_artifact.lineage[1].source_index == 1);
	assert(merged_artifact.lineage[1].source_id == right.space.id(0));

	auto restored_merged = mtrc::space::persistence::restore_merged_space(merged_artifact, metric_type{});
	const auto first_origin = mtrc::space::merge_origin(restored_merged, restored_merged.space.id(0));
	const auto second_origin = mtrc::space::merge_origin(restored_merged, restored_merged.space.id(1));
	assert(first_origin.source_index == 0);
	assert(first_origin.source_id == left.space.id(0));
	assert(second_origin.source_index == 1);
	assert(second_origin.source_id == right.space.id(0));

	std::stringstream merged_stream;
	mtrc::space::persistence::save(merged_stream, merged);
	const auto loaded_merged = mtrc::space::persistence::load_merged_space<std::string>(merged_stream, metric_type{});
	assert(mtrc::space::merge_origin(loaded_merged.merged, loaded_merged.merged.space.id(2)).source_index == 1);

	auto bad_merged_artifact = merged_artifact;
	bad_merged_artifact.lineage.front().local_id = merged.space.id(1);
	assert(throws<mtrc::MetricInputError>(
		[&] { (void)mtrc::space::persistence::restore_merged_space(bad_merged_artifact, metric_type{}); }));

	return 0;
}
