#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "metric/distance.hpp"
#include "metric/engine.hpp"

namespace {

auto close(double lhs, double rhs) -> bool
{
	return std::fabs(lhs - rhs) < 1e-9;
}

} // namespace

int main()
{
	using record_type = std::vector<double>;

	auto space = metric::make_space(std::vector<record_type>{{0, 1, 2}, {0, 1, 3}}, metric::Euclidean<double>{});

	const auto embedding = metric::embed(space, metric::strategies::pcfa(2));
	assert(embedding.mapping == "pcfa_embedding");
	assert(embedding.strategy == "pcfa");
	assert(embedding.representation == "metric_space");
	assert(embedding.inverse_supported);
	assert(embedding.source_record_count == space.size());
	assert(embedding.source_records.size() == space.size());
	assert(embedding.representative_records.size() == space.size());
	assert(embedding.space.size() == space.size());
	assert(embedding.space.record(embedding.space.id(0)).size() == 2);
	assert(close(embedding.space.record(embedding.space.id(0))[0], -0.5));
	assert(close(embedding.space.record(embedding.space.id(1))[0], 0.5));

	const auto direct = metric::embed(space, 2);
	assert(direct.mapping == "pcfa_embedding");
	assert(direct.source_records == embedding.source_records);
	assert(close(direct.space.record(direct.space.id(0))[0], embedding.space.record(embedding.space.id(0))[0]));

	const auto lazy_policy = metric::runtime::exact();
	const auto runtime_embedding = metric::embed(space, metric::strategies::pcfa(2), lazy_policy);
	assert(runtime_embedding.mapping == "pcfa_embedding");
	assert(runtime_embedding.representation == "metric_space");
	assert(runtime_embedding.source_records == embedding.source_records);
	assert(close(runtime_embedding.space.record(runtime_embedding.space.id(0))[0],
				 embedding.space.record(embedding.space.id(0))[0]));

	const auto runtime_direct = metric::embed(space, 2, lazy_policy);
	assert(runtime_direct.representation == "metric_space");
	assert(runtime_direct.source_records == embedding.source_records);

	bool rejected_zero_components = false;
	try {
		(void)metric::embed(space, 0);
	} catch (const std::invalid_argument &) {
		rejected_zero_components = true;
	}
	assert(rejected_zero_components);

	bool rejected_materialized_runtime = false;
	try {
		(void)metric::embed(space, 2, metric::runtime::materialized(metric::runtime::exact()));
	} catch (const std::invalid_argument &) {
		rejected_materialized_runtime = true;
	}
	assert(rejected_materialized_runtime);

	bool rejected_approximate_runtime = false;
	try {
		(void)metric::embed(space, 2, metric::runtime::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	return 0;
}
