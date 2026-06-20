#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/distance.hpp"
#include "metric/engine.hpp"

struct LengthDelta {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> std::size_t
	{
		return lhs.size() > rhs.size() ? lhs.size() - rhs.size() : rhs.size() - lhs.size();
	}
};

int main()
{
	std::vector<std::string> records = {"metric", "metrics", "matrix", "tree"};

	static_assert(metric::MetricCallable_v<metric::Edit<char>, std::string>);
	static_assert(metric::MetricCallable_v<LengthDelta, std::string>);

	metric::MetricSpace<std::string, metric::Edit<char>> typed_space(records, metric::Edit<char>{});
	static_assert(metric::MetricSpaceLike_v<decltype(typed_space)>);

	const auto metric_id = typed_space.id(0);
	const auto metrics_id = typed_space.id(1);
	static_assert(std::is_same<decltype(typed_space.version()), metric::SpaceVersion>::value);
	assert(metric::initial_space_version == 0);
	assert(metric::next_space_version(metric::initial_space_version) == 1);
	assert(metric_id.index() == 0);
	assert(typed_space.size() == records.size());
	assert(!typed_space.empty());
	assert(typed_space.version() == 0);
	assert(typed_space.id_at(0) == metric_id);
	assert(typed_space.contains(metric_id));
	assert(typed_space.position_of(metrics_id) == 1);
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
	bool rejected_erased_id = false;
	try {
		(void)typed_space.position_of(metrics_id);
	} catch (const std::out_of_range &) {
		rejected_erased_id = true;
	}
	assert(rejected_erased_id);
	assert(!typed_space.erase(metrics_id));

	const auto made_space = metric::make_space(records, metric::Edit<char>{});
	static_assert(metric::MetricSpaceLike_v<decltype(made_space)>);
	assert(made_space.distance(made_space.id(0), made_space.id(1)) == 1);
	assert(made_space.version() == 0);

	const auto custom_space = metric::make_space(records, LengthDelta{});
	assert(custom_space.distance(custom_space.id(0), custom_space.id(3)) == 2);
	assert(metric::metric_traits<LengthDelta>::law == metric::metric_law::distance);
	assert(metric::metric_traits<LengthDelta>::records == metric::record_kind::custom);

	bool rejected_bad_id = false;
	try {
		(void)custom_space.id(records.size());
	} catch (const std::out_of_range &) {
		rejected_bad_id = true;
	}
	assert(rejected_bad_id);

	return 0;
}
