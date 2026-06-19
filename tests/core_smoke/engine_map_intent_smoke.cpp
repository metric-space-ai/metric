#include <metric/distance.hpp>
#include <metric/engine.hpp>

#include <cassert>
#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

int main()
{
	std::vector<std::string> records = {"metric", "map", "engine"};
	auto space = metric::make_space(records, metric::Edit<char>{});

	auto length_distance = [](std::size_t lhs, std::size_t rhs) -> std::size_t {
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	};
	auto mapped = metric::map(
		space, [](const std::string &record) -> std::size_t { return record.size(); }, length_distance);

	using mapped_type = typename std::decay<decltype(mapped)>::type;
	static_assert(std::is_same<typename mapped_type::space_type::record_type, std::size_t>::value,
				  "map should produce a target record type from the transform");

	assert(mapped.mapping == "deterministic_transform");
	assert(mapped.strategy == "deterministic_transform");
	assert(mapped.representation == "metric_space");
	assert(!mapped.inverse_supported);
	assert(mapped.source_record_count == space.size());
	assert(mapped.source_records.size() == space.size());
	assert(mapped.representative_records.size() == space.size());
	assert(mapped.space.size() == space.size());
	assert(mapped.space.record(mapped.space.id(0)) == 6);
	assert(mapped.space.record(mapped.space.id(1)) == 3);
	assert(mapped.space.distance(mapped.space.id(0), mapped.space.id(1)) == 3);

	for (std::size_t index = 0; index < space.size(); ++index) {
		assert(mapped.source_records[index].size() == 1);
		assert(mapped.source_records[index][0] == space.id(index));
		assert(mapped.representative_records[index] == space.id(index));
	}

	return 0;
}
