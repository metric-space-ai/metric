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

	bool rejected_zero_components = false;
	try {
		(void)metric::reduce(space, 0);
	} catch (const std::invalid_argument &) {
		rejected_zero_components = true;
	}
	assert(rejected_zero_components);

	return 0;
}
