#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

int main()
{
	std::vector<std::string> records = {"metric", "map", "engine"};
	auto space = mtrc::make_space(records, mtrc::Edit<char>{});

	auto manual_space = mtrc::make_space(std::vector<std::size_t>{6, 3}, [](std::size_t lhs, std::size_t rhs) {
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	});
	const auto manual = mtrc::core::make_mapping_result(
		std::move(manual_space), std::vector<std::vector<mtrc::RecordId>>{{space.id(0)}, {space.id(1)}},
		std::vector<mtrc::RecordId>{space.id(0), space.id(1)}, space.size(), true, "manual_mapping",
		"manual_strategy", "manual_representation");
	assert(manual.size() == 2);
	assert(!manual.empty());
	assert(manual.source_record_count == space.size());
	assert(manual.inverse_supported);
	assert(manual.mapping == "manual_mapping");
	assert(manual.strategy == "manual_strategy");
	assert(manual.representation == "manual_representation");
	assert(manual.source_records[0][0] == space.id(0));
	assert(manual.representative_records[1] == space.id(1));

	auto length_distance = [](std::size_t lhs, std::size_t rhs) -> std::size_t {
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	};
	auto mapped =
		mtrc::map(space, [](const std::string &record) -> std::size_t { return record.size(); }, length_distance);

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

	const auto runtime_mapped = mtrc::map(
		space, [](const std::string &record) -> std::size_t { return record.size(); }, length_distance,
		mtrc::space::storage::exact());
	assert(runtime_mapped.representation == "metric_space");
	assert(runtime_mapped.source_records == mapped.source_records);
	assert(runtime_mapped.space.record(runtime_mapped.space.id(0)) == 6);

	bool rejected_materialized_runtime = false;
	try {
		(void)mtrc::map(
			space, [](const std::string &record) -> std::size_t { return record.size(); }, length_distance,
			mtrc::space::storage::materialized(mtrc::space::storage::exact()));
	} catch (const std::invalid_argument &) {
		rejected_materialized_runtime = true;
	}
	assert(rejected_materialized_runtime);

	bool rejected_approximate_runtime = false;
	try {
		(void)mtrc::map(
			space, [](const std::string &record) -> std::size_t { return record.size(); }, length_distance,
			mtrc::space::storage::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	return 0;
}
