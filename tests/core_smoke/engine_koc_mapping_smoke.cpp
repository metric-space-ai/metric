#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"
#include "metric/modify/map/koc.hpp"

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

	auto space = mtrc::make_space(std::vector<record_type>{{0.0, 0.0}, {1.0, 1.0}}, mtrc::Euclidean<double>{});
	auto mapping = mtrc::modify::map::koc(2);
	static_assert(mtrc::Mapping_v<decltype(mapping), decltype(space)>);
	assert(mapping.clusters() == 2);

	using model_type = mtrc::modify::map::KOCModel<decltype(space)>;
	static_assert(mtrc::MappingModel_v<model_type, decltype(space)>);
	model_type model(2);
	assert(model.clusters() == 2);
	assert(!model.inverse_supported());

	bool rejected_fit = false;
	try {
		(void)mtrc::modify::map::fit(mapping, space);
	} catch (const std::invalid_argument &error) {
		rejected_fit = is_unpromoted_koc_error(error);
	}
	assert(rejected_fit);

	bool rejected_transform = false;
	try {
		(void)mtrc::modify::map::transform(model, space);
	} catch (const std::invalid_argument &error) {
		rejected_transform = is_unpromoted_koc_error(error);
	}
	assert(rejected_transform);

	bool rejected_map = false;
	try {
		(void)mtrc::map(space, mapping);
	} catch (const std::invalid_argument &error) {
		rejected_map = is_unpromoted_koc_error(error);
	}
	assert(rejected_map);

	bool rejected_invalid_clusters = false;
	try {
		(void)mtrc::modify::map::koc(0);
	} catch (const std::invalid_argument &) {
		rejected_invalid_clusters = true;
	}
	assert(rejected_invalid_clusters);

	return 0;
}
