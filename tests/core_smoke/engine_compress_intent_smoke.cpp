#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

int main()
{
	const auto line = metric::make_space(std::vector<int>{0, 1, 2, 3, 4}, AbsoluteDistance{});

	const auto compressed = metric::compress(line, 3);
	assert(compressed.source_record_count == 5);
	assert(compressed.compressed_record_count == 3);
	assert(compressed.size() == 3);
	assert((compressed.source_record_ids == std::vector<metric::RecordId>{line.id(0), line.id(4), line.id(2)}));
	assert((compressed.assignments == std::vector<std::size_t>{0, 0, 2, 2, 1}));
	assert((compressed.nearest_representative_distances == std::vector<int>{0, 1, 0, 1, 0}));
	assert(compressed.space.record(compressed.space.id(0)) == 0);
	assert(compressed.space.record(compressed.space.id(1)) == 4);
	assert(compressed.space.record(compressed.space.id(2)) == 2);
	assert(std::abs(compressed.compression_ratio - 0.6) < 1e-12);
	assert(compressed.exact);
	assert(compressed.operator_name == "compress");
	assert(compressed.compression == "representatives");
	assert(compressed.strategy == "farthest_first");
	assert(compressed.representation == "metric_space");
	assert(compressed.lossy);
	assert(!compressed.inverse_supported);

	const auto materialized_policy = metric::runtime::materialized(metric::runtime::exact());
	const auto materialized = metric::compress(line, 3, materialized_policy);
	assert(materialized.representation == "matrix_cache");
	assert(materialized.source_record_ids == compressed.source_record_ids);
	assert(materialized.assignments == compressed.assignments);
	assert(materialized.nearest_representative_distances == compressed.nearest_representative_distances);
	assert(materialized.space.record(materialized.space.id(1)) == compressed.space.record(compressed.space.id(1)));

	const auto seeded = metric::compress(line, 2, metric::strategies::farthest_first(2));
	assert((seeded.source_record_ids == std::vector<metric::RecordId>{line.id(2), line.id(0)}));
	assert((seeded.assignments == std::vector<std::size_t>{1, 1, 0, 0, 0}));

	const auto materialized_seeded =
		metric::compress(line, 2, metric::strategies::farthest_first(2), materialized_policy);
	assert(materialized_seeded.representation == "matrix_cache");
	assert(materialized_seeded.source_record_ids == seeded.source_record_ids);
	assert(materialized_seeded.assignments == seeded.assignments);

	bool rejected_zero = false;
	try {
		(void)metric::compress(line, 0);
	} catch (const std::invalid_argument &) {
		rejected_zero = true;
	}
	assert(rejected_zero);

	bool rejected_empty = false;
	try {
		const auto empty = metric::make_space(std::vector<int>{}, AbsoluteDistance{});
		(void)metric::compress(empty, 1);
	} catch (const std::invalid_argument &) {
		rejected_empty = true;
	}
	assert(rejected_empty);

	bool rejected_too_many = false;
	try {
		(void)metric::compress(line, line.size() + 1);
	} catch (const std::invalid_argument &) {
		rejected_too_many = true;
	}
	assert(rejected_too_many);

	bool rejected_approximate_runtime = false;
	try {
		(void)metric::compress(line, 2, metric::runtime::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	return 0;
}
