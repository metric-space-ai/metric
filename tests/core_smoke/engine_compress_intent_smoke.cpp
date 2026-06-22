#include <cassert>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

int main()
{
	const auto line = mtrc::make_space(std::vector<int>{0, 1, 2, 3, 4}, AbsoluteDistance{});

	auto manual_space = mtrc::make_space(std::vector<int>{0, 4}, AbsoluteDistance{});
	const auto manual = mtrc::core::make_compression_result(
		std::move(manual_space), std::vector<mtrc::RecordId>{line.id(0), line.id(4)},
		std::vector<std::size_t>{0, 1, 1}, std::vector<int>{0, 2, 1}, 3, "manual_compression",
		"manual_strategy", "manual_provider", false, true, false);
	assert(manual.source_record_count == 3);
	assert(manual.compressed_record_count == 2);
	assert(manual.size() == 2);
	assert((manual.source_record_ids == std::vector<mtrc::RecordId>{line.id(0), line.id(4)}));
	assert((manual.assignments == std::vector<std::size_t>{0, 1, 1}));
	assert((manual.nearest_representative_distances == std::vector<int>{0, 2, 1}));
	assert(std::abs(manual.compression_ratio - (2.0 / 3.0)) < 1e-12);
	assert(!manual.exact);
	assert(manual.operator_name == "compress");
	assert(manual.compression == "manual_compression");
	assert(manual.strategy == "manual_strategy");
	assert(manual.representation == "manual_provider");
	assert(manual.lossy);
	assert(!manual.inverse_supported);

	auto empty_manual_space = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
	const auto empty_manual = mtrc::core::make_compression_result(
		std::move(empty_manual_space), std::vector<mtrc::RecordId>{}, std::vector<std::size_t>{},
		std::vector<int>{}, 0, "empty_compression", "empty_strategy", "empty_provider");
	assert(empty_manual.empty());
	assert(empty_manual.source_record_count == 0);
	assert(empty_manual.compressed_record_count == 0);
	assert(empty_manual.compression_ratio == 0.0);
	assert(empty_manual.exact);
	assert(empty_manual.lossy);
	assert(!empty_manual.inverse_supported);

	const auto compressed = mtrc::compress(line, 3);
	assert(compressed.source_record_count == 5);
	assert(compressed.compressed_record_count == 3);
	assert(compressed.size() == 3);
	assert((compressed.source_record_ids == std::vector<mtrc::RecordId>{line.id(0), line.id(4), line.id(2)}));
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

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto materialized = mtrc::compress(line, 3, materialized_policy);
	assert(materialized.representation == "distance_table");
	assert(materialized.source_record_ids == compressed.source_record_ids);
	assert(materialized.assignments == compressed.assignments);
	assert(materialized.nearest_representative_distances == compressed.nearest_representative_distances);
	assert(materialized.space.record(materialized.space.id(1)) == compressed.space.record(compressed.space.id(1)));

	const auto seeded = mtrc::compress(line, 2, mtrc::space::select::farthest_first(2));
	assert((seeded.source_record_ids == std::vector<mtrc::RecordId>{line.id(2), line.id(0)}));
	assert((seeded.assignments == std::vector<std::size_t>{1, 1, 0, 0, 0}));

	const auto materialized_seeded =
		mtrc::compress(line, 2, mtrc::space::select::farthest_first(2), materialized_policy);
	assert(materialized_seeded.representation == "distance_table");
	assert(materialized_seeded.source_record_ids == seeded.source_record_ids);
	assert(materialized_seeded.assignments == seeded.assignments);

	bool rejected_zero = false;
	try {
		(void)mtrc::compress(line, 0);
	} catch (const std::invalid_argument &) {
		rejected_zero = true;
	}
	assert(rejected_zero);

	bool rejected_empty = false;
	try {
		const auto empty = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
		(void)mtrc::compress(empty, 1);
	} catch (const std::invalid_argument &) {
		rejected_empty = true;
	}
	assert(rejected_empty);

	bool rejected_too_many = false;
	try {
		(void)mtrc::compress(line, line.size() + 1);
	} catch (const std::invalid_argument &) {
		rejected_too_many = true;
	}
	assert(rejected_too_many);

	bool rejected_approximate_runtime = false;
	try {
		(void)mtrc::compress(line, 2, mtrc::space::storage::approximate());
	} catch (const std::invalid_argument &) {
		rejected_approximate_runtime = true;
	}
	assert(rejected_approximate_runtime);

	return 0;
}
