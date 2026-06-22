#include <cassert>
#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"
#include "metric/modify/map/pcfa.hpp"

namespace {

auto close(double lhs, double rhs) -> bool { return std::fabs(lhs - rhs) < 1e-9; }

} // namespace

int main()
{
	using record_type = std::vector<double>;

	auto space = mtrc::make_space(std::vector<record_type>{{0, 1, 2}, {0, 1, 3}}, mtrc::Euclidean<double>{});
	auto mapping = mtrc::modify::map::pcfa(2);
	static_assert(mtrc::Mapping_v<decltype(mapping), decltype(space)>);

	auto model = mtrc::modify::map::fit(mapping, space);
	static_assert(mtrc::MappingModel_v<decltype(model), decltype(space)>);
	assert(model.component_count() == 2);

	const auto reduced = mtrc::modify::map::transform(model, space);
	using reduced_type = typename std::decay<decltype(reduced)>::type;
	static_assert(std::is_same<typename reduced_type::space_type::record_type, record_type>::value);
	assert(reduced.mapping == "pcfa");
	assert(reduced.strategy == "pcfa");
	assert(reduced.representation == "metric_space");
	assert(reduced.inverse_supported);
	assert(reduced.source_record_count == space.size());
	assert(reduced.source_records.size() == space.size());
	assert(reduced.representative_records[0] == space.id(0));
	assert(reduced.space.size() == space.size());
	assert(reduced.space.record(reduced.space.id(0)).size() == 2);
	assert(close(reduced.space.record(reduced.space.id(0))[0], -0.5));
	assert(close(reduced.space.record(reduced.space.id(0))[1], 0.0));
	assert(close(reduced.space.record(reduced.space.id(1))[0], 0.5));
	assert(close(reduced.space.record(reduced.space.id(1))[1], 0.0));

	const auto restored = model.inverse_transform(reduced);
	assert(restored.size() == space.size());
	for (std::size_t index = 0; index < restored.size(); ++index) {
		assert(restored[index].size() == space.record(space.id(index)).size());
		for (std::size_t dimension = 0; dimension < restored[index].size(); ++dimension) {
			assert(close(restored[index][dimension], space.record(space.id(index))[dimension]));
		}
	}

	const auto direct = mtrc::modify::map::pcfa_space(space, 2);
	assert(direct.space.size() == reduced.space.size());
	assert(direct.source_records == reduced.source_records);

	const auto intent_mapped = mtrc::map(space, mapping);
	using intent_mapped_type = typename std::decay<decltype(intent_mapped)>::type;
	static_assert(std::is_same<typename intent_mapped_type::space_type::record_type, record_type>::value,
				  "map intent should produce the mapping adapter target record type");
	assert(intent_mapped.mapping == "pcfa");
	assert(intent_mapped.strategy == "pcfa");
	assert(intent_mapped.representation == "metric_space");
	assert(intent_mapped.inverse_supported);
	assert(intent_mapped.source_record_count == space.size());
	assert(intent_mapped.source_records == reduced.source_records);
	assert(intent_mapped.space.size() == reduced.space.size());
	assert(close(intent_mapped.space.record(intent_mapped.space.id(0))[0], -0.5));
	assert(close(intent_mapped.space.record(intent_mapped.space.id(1))[0], 0.5));

	const auto runtime_intent_mapped = mtrc::map(space, mapping, mtrc::space::storage::exact());
	assert(runtime_intent_mapped.mapping == "pcfa");
	assert(runtime_intent_mapped.representation == "metric_space");
	assert(runtime_intent_mapped.source_records == intent_mapped.source_records);

	bool rejected_zero_components = false;
	try {
		(void)mtrc::modify::map::pcfa(0);
	} catch (const std::invalid_argument &) {
		rejected_zero_components = true;
	}
	assert(rejected_zero_components);

	return 0;
}
