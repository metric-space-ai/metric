#include <cassert>
#include <iostream>
#include <vector>

#include <metric/workflow.hpp>

namespace {

auto mean(const std::vector<double> &values) -> double
{
	double sum = 0.0;
	for (const auto value : values) {
		sum += value;
	}
	return values.empty() ? 0.0 : sum / static_cast<double>(values.size());
}

} // namespace

int main()
{
	using curve = std::vector<double>;
	using coordinates = std::vector<double>;

	const std::vector<curve> curves = {
		{0.0, 0.1, 0.2, 0.1},
		{0.1, 0.2, 0.4, 0.2},
		{0.6, 0.8, 1.0, 0.8},
		{0.7, 0.9, 1.1, 0.9},
	};

	auto source = mtrc::space::build_checked(curves, mtrc::TWED<double>(0.02, 1.0));

	const auto feature_map = [](const curve &record) -> coordinates {
		return {mean(record), record.back() - record.front()};
	};

	auto derived = mtrc::modify::map::map(source, feature_map, mtrc::Euclidean<double>{});
	assert(derived.size() == source.size());
	assert(derived.source_record_count == source.size());
	assert(derived.metric_status == mtrc::metric_law::metric);
	assert(derived.out_of_sample_supported);

	const coordinates probe = feature_map(curve{0.68, 0.88, 1.08, 0.88});
	const auto nearest = mtrc::space::query::nearest(derived.space, probe);
	assert(nearest.id == derived.space.id(3));

	std::cout << "derived coordinates=" << derived.size() << "\n";
	std::cout << "nearest derived id=" << nearest.id.index() << "\n";
	return 0;
}
