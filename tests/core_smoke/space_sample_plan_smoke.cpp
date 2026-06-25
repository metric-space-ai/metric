#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/modify/reduce/compress.hpp"
#include "metric/space/sample_plan.hpp"
#include "metric/space/storage/execution.hpp"
#include "metric/stats/search/neighbors.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct CountingAbsoluteDistance {
	std::shared_ptr<std::size_t> calls;

	auto operator()(int lhs, int rhs) const -> int
	{
		++*calls;
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

template <typename Callable> static auto throws_out_of_range(Callable &&call) -> bool
{
	try {
		call();
	} catch (const std::out_of_range &) {
		return true;
	}
	return false;
}

int main()
{
	const auto regular = mtrc::space::regular_sample_positions(10, 4);
	assert((regular.positions == std::vector<std::size_t>{0, 2, 5, 7}));
	assert(regular.record_count == 10);
	assert(regular.requested_count == 4);
	assert(regular.candidate_universe == 10);
	assert(!regular.excludes_position);
	assert(regular.size() == 4);

	const auto clamped = mtrc::space::regular_sample_positions(3, 9);
	assert((clamped.positions == std::vector<std::size_t>{0, 1, 2}));
	assert(clamped.requested_count == 9);
	assert(clamped.candidate_universe == 3);

	const auto excluding = mtrc::space::regular_sample_positions_excluding(6, 2, 5);
	assert((excluding.positions == std::vector<std::size_t>{0, 1, 3, 4, 5}));
	assert(excluding.candidate_universe == 5);
	assert(excluding.excludes_position);
	assert(excluding.excluded_position == 2);

	const auto sparse_excluding = mtrc::space::regular_sample_positions_excluding(6, 2, 3);
	assert((sparse_excluding.positions == std::vector<std::size_t>{0, 1, 4}));
	assert(throws_out_of_range([] { (void)mtrc::space::regular_sample_positions_excluding(3, 3, 1); }));

	auto small_space = mtrc::make_space(std::vector<int>{0, 10, 20, 30, 40, 50, 60, 70, 80, 90},
										AbsoluteDistance{});
	const auto sampled_ids = mtrc::space::record_ids_for_sample_plan(small_space, regular);
	assert((sampled_ids == std::vector<mtrc::RecordId>{small_space.id(0), small_space.id(2),
													   small_space.id(5), small_space.id(7)}));

	std::vector<int> records;
	records.reserve(4101);
	for (int value = 0; value < 4101; ++value) {
		records.push_back(value);
	}
	auto calls = std::make_shared<std::size_t>(0);
	auto search_space = mtrc::make_space(records, CountingAbsoluteDistance{calls});
	const auto approximate_policy =
		mtrc::space::storage::using_knn_graph(4, mtrc::space::storage::approximate());
	const auto query_id = search_space.id(2);
	const auto query_plan = mtrc::space::regular_sample_positions_excluding(search_space.size(), 2, 4);

	const auto approximate_neighbors = mtrc::find_neighbors(search_space, query_id, 2, approximate_policy);
	assert(!approximate_neighbors.exact);
	assert(approximate_neighbors.representation == "sampled_metric_space");
	assert(approximate_neighbors.approximation_quality.candidate_count == query_plan.size());
	assert(approximate_neighbors.approximation_quality.candidate_universe == query_plan.candidate_universe);
	assert(!approximate_neighbors.approximation_quality.recall_measured);
	assert(*calls == query_plan.size());
	assert(approximate_neighbors.size() == 2);
	assert(approximate_neighbors[0].id == search_space.id(query_plan.positions[0]));
	assert(approximate_neighbors[1].id == search_space.id(query_plan.positions[1]));

	*calls = 0;
	const auto approximate_range = mtrc::stats::search::range(search_space, query_id, 3, approximate_policy);
	assert(!approximate_range.exact);
	assert(approximate_range.representation == "sampled_metric_space");
	assert(approximate_range.approximation_quality.candidate_count == query_plan.size());
	assert(approximate_range.size() == 1);
	assert(approximate_range[0].id == search_space.id(query_plan.positions[0]));
	assert(*calls == query_plan.size());

	auto compress_space = mtrc::make_space(std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7}, AbsoluteDistance{});
	const auto compress_plan = mtrc::space::regular_sample_positions(compress_space.size(), 4);
	assert((compress_plan.positions == std::vector<std::size_t>{0, 2, 4, 6}));
	const auto compressed =
		mtrc::compress(compress_space, 2, mtrc::space::select::farthest_first{}, approximate_policy);
	assert(!compressed.exact);
	assert(compressed.representation == "sampled_metric_space");
	assert(compressed.strategy == "sampled_farthest_first");
	assert((compressed.source_record_ids ==
			std::vector<mtrc::RecordId>{compress_space.id(compress_plan.positions[0]), compress_space.id(6)}));

	return 0;
}
