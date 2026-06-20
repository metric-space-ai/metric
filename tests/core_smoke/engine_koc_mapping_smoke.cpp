#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/distance.hpp"
#include "metric/engine.hpp"
#include "metric/mappings/koc.hpp"

namespace {

auto is_unpromoted_koc_error(const std::invalid_argument &error) -> bool
{
	const std::string message = error.what();
	return message.find("KOC") != std::string::npos && message.find("not promoted") != std::string::npos;
}

} // namespace

int main()
{
	using record_type = std::vector<double>;

	auto space = metric::make_space(std::vector<record_type>{{0.0, 0.0}, {1.0, 1.0}}, metric::Euclidean<double>{});
	auto mapping = metric::mappings::koc(2);
	static_assert(metric::Mapping_v<decltype(mapping), decltype(space)>);
	assert(mapping.clusters() == 2);

	using model_type = metric::mappings::KOCModel<decltype(space)>;
	static_assert(metric::MappingModel_v<model_type, decltype(space)>);
	model_type model(2);
	assert(model.clusters() == 2);
	assert(!model.inverse_supported());

	bool rejected_fit = false;
	try {
		(void)metric::mappings::fit(mapping, space);
	} catch (const std::invalid_argument &error) {
		rejected_fit = is_unpromoted_koc_error(error);
	}
	assert(rejected_fit);

	bool rejected_transform = false;
	try {
		(void)metric::mappings::transform(model, space);
	} catch (const std::invalid_argument &error) {
		rejected_transform = is_unpromoted_koc_error(error);
	}
	assert(rejected_transform);

	bool rejected_map = false;
	try {
		(void)metric::map(space, mapping);
	} catch (const std::invalid_argument &error) {
		rejected_map = is_unpromoted_koc_error(error);
	}
	assert(rejected_map);

	bool rejected_invalid_clusters = false;
	try {
		(void)metric::mappings::koc(0);
	} catch (const std::invalid_argument &) {
		rejected_invalid_clusters = true;
	}
	assert(rejected_invalid_clusters);

	return 0;
}
