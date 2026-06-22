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

	return 0;
}
