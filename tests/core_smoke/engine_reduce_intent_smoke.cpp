#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"

namespace {

auto close(double lhs, double rhs) -> bool { return std::fabs(lhs - rhs) < 1e-9; }

} // namespace

int main()
{
	using record_type = std::vector<double>;

	auto space = mtrc::make_space(std::vector<record_type>{{0, 1, 2}, {0, 1, 3}}, mtrc::Euclidean<double>{});

	const auto reduced = mtrc::reduce(space, mtrc::modify::map::pcfa_options(2));
	assert(reduced.mapping == "pcfa");
	assert(reduced.strategy == "pcfa");
	assert(reduced.inverse_supported);
	assert(reduced.source_record_count == space.size());
	assert(reduced.space.size() == space.size());
	assert(reduced.space.record(reduced.space.id(0)).size() == 2);
	assert(close(reduced.space.record(reduced.space.id(0))[0], -0.5));
	assert(close(reduced.space.record(reduced.space.id(1))[0], 0.5));

	const auto direct = mtrc::reduce(space, 2);
	assert(direct.source_records == reduced.source_records);
	assert(close(direct.space.record(direct.space.id(0))[0], reduced.space.record(reduced.space.id(0))[0]));

	const auto lazy_policy = mtrc::space::storage::exact();
	const auto runtime_reduced = mtrc::reduce(space, mtrc::modify::map::pcfa_options(2), lazy_policy);
	assert(runtime_reduced.representation == "metric_space");
	assert(runtime_reduced.source_records == reduced.source_records);
	assert(close(runtime_reduced.space.record(runtime_reduced.space.id(0))[0],
				 reduced.space.record(reduced.space.id(0))[0]));

	const auto runtime_direct = mtrc::reduce(space, 2, lazy_policy);
	assert(runtime_direct.representation == "metric_space");
	assert(runtime_direct.source_records == reduced.source_records);

	bool rejected_zero_components = false;
	try {
		(void)mtrc::reduce(space, 0);
	} catch (const std::invalid_argument &) {
		rejected_zero_components = true;
	}
	assert(rejected_zero_components);

	bool rejected_materialized_runtime = false;
	try {
		(void)mtrc::reduce(space, 2, mtrc::space::storage::materialized(mtrc::space::storage::exact()));
	} catch (const std::invalid_argument &) {
		rejected_materialized_runtime = true;
	}
	assert(rejected_materialized_runtime);

	bool rejected_approximate_runtime = false;
	try {
		(void)mtrc::reduce(space, 2, mtrc::space::storage::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	return 0;
}
