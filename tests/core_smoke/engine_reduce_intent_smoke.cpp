#include <cassert>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/distance.hpp"
#include "metric/engine.hpp"

namespace {

auto close(double lhs, double rhs) -> bool
{
	return std::fabs(lhs - rhs) < 1e-9;
}

template <typename Space, typename Strategy>
auto rejects_unpromoted_strategy(const Space &space, Strategy strategy, const std::string &name) -> bool
{
	try {
		(void)metric::reduce(space, strategy);
	} catch (const std::invalid_argument &error) {
		const std::string message = error.what();
		return message.find(name) != std::string::npos && message.find("not promoted") != std::string::npos;
	}
	return false;
}

} // namespace

int main()
{
	using record_type = std::vector<double>;

	auto space = metric::make_space(std::vector<record_type>{{0, 1, 2}, {0, 1, 3}}, metric::Euclidean<double>{});

	const auto reduced = metric::reduce(space, metric::strategies::pcfa(2));
	assert(reduced.mapping == "pcfa");
	assert(reduced.strategy == "pcfa");
	assert(reduced.inverse_supported);
	assert(reduced.source_record_count == space.size());
	assert(reduced.space.size() == space.size());
	assert(reduced.space.record(reduced.space.id(0)).size() == 2);
	assert(close(reduced.space.record(reduced.space.id(0))[0], -0.5));
	assert(close(reduced.space.record(reduced.space.id(1))[0], 0.5));

	const auto direct = metric::reduce(space, 2);
	assert(direct.source_records == reduced.source_records);
	assert(close(direct.space.record(direct.space.id(0))[0], reduced.space.record(reduced.space.id(0))[0]));

	const auto lazy_policy = metric::runtime::exact();
	const auto runtime_reduced = metric::reduce(space, metric::strategies::pcfa(2), lazy_policy);
	assert(runtime_reduced.representation == "metric_space");
	assert(runtime_reduced.source_records == reduced.source_records);
	assert(close(runtime_reduced.space.record(runtime_reduced.space.id(0))[0],
				 reduced.space.record(reduced.space.id(0))[0]));

	const auto runtime_direct = metric::reduce(space, 2, lazy_policy);
	assert(runtime_direct.representation == "metric_space");
	assert(runtime_direct.source_records == reduced.source_records);

	bool rejected_zero_components = false;
	try {
		(void)metric::reduce(space, 0);
	} catch (const std::invalid_argument &) {
		rejected_zero_components = true;
	}
	assert(rejected_zero_components);

	bool rejected_materialized_runtime = false;
	try {
		(void)metric::reduce(space, 2, metric::runtime::materialized(metric::runtime::exact()));
	} catch (const std::invalid_argument &) {
		rejected_materialized_runtime = true;
	}
	assert(rejected_materialized_runtime);

	bool rejected_approximate_runtime = false;
	try {
		(void)metric::reduce(space, 2, metric::runtime::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	const auto som_strategy = metric::strategies::som(2, 2);
	assert(som_strategy.width == 2);
	assert(som_strategy.height == 2);
	assert(rejects_unpromoted_strategy(space, som_strategy, "SOM"));

	const auto koc_strategy = metric::strategies::koc(2);
	assert(koc_strategy.clusters == 2);
	assert(rejects_unpromoted_strategy(space, koc_strategy, "KOC"));

	const auto dspcc_strategy = metric::strategies::dspcc(2);
	assert(dspcc_strategy.components == 2);
	assert(rejects_unpromoted_strategy(space, dspcc_strategy, "DSPCC"));

	bool rejected_invalid_som = false;
	try {
		(void)metric::strategies::som(0, 2);
	} catch (const std::invalid_argument &) {
		rejected_invalid_som = true;
	}
	assert(rejected_invalid_som);

	bool rejected_invalid_koc = false;
	try {
		(void)metric::strategies::koc(0);
	} catch (const std::invalid_argument &) {
		rejected_invalid_koc = true;
	}
	assert(rejected_invalid_koc);

	bool rejected_invalid_dspcc = false;
	try {
		(void)metric::strategies::dspcc(0);
	} catch (const std::invalid_argument &) {
		rejected_invalid_dspcc = true;
	}
	assert(rejected_invalid_dspcc);

	return 0;
}
