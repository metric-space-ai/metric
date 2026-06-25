// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// User-facing search and sample workflows: batch kNN/range queries (one result per
// query, identical to the single-query overload) and the regular_sample lineage result.
// Both are read-only investigations of an existing finite metric space.

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/core/result.hpp"
#include "metric/space/storage/implicit.hpp"
#include "metric/stats/sample/sample.hpp"
#include "metric/stats/search/nearest.hpp"
#include "metric/stats/search/neighbors.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

template <typename Set> auto neighbor_sets_equal(const Set &lhs, const Set &rhs) -> bool
{
	if (lhs.size() != rhs.size()) {
		return false;
	}
	for (std::size_t index = 0; index < lhs.size(); ++index) {
		if (lhs[index].id != rhs[index].id || lhs[index].distance != rhs[index].distance) {
			return false;
		}
	}
	return true;
}

template <typename Function> auto throws_invalid_argument(Function run) -> bool
{
	try {
		run();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

template <typename Function> auto throws_out_of_range(Function run) -> bool
{
	try {
		run();
	} catch (const std::out_of_range &) {
		return true;
	}
	return false;
}

void knn_batch_matches_single_queries()
{
	namespace search = mtrc::stats::search;
	const auto space = mtrc::make_space(std::vector<int>{0, 4, 8, 12}, AbsoluteDistance{});

	// RecordId batch == per-id single query (self excluded, RecordId tie policy preserved).
	const std::vector<mtrc::RecordId> ids = {space.id(0), space.id(1)};
	const auto id_batch = search::knn_batch(space, ids, 1);
	assert(id_batch.size() == 2);
	assert(neighbor_sets_equal(id_batch[0], search::knn(space, space.id(0), 1)));
	assert(neighbor_sets_equal(id_batch[1], search::knn(space, space.id(1), 1)));
	assert(mtrc::summary(id_batch[0]).find("NeighborSet") != std::string::npos);

	// Value batch == per-value single query (no self-exclusion for value queries).
	const std::vector<int> queries = {0, 8};
	const auto value_batch = search::knn_batch(space, queries, 2);
	assert(value_batch.size() == 2);
	assert(neighbor_sets_equal(value_batch[0], search::knn(space, 0, 2)));
	assert(neighbor_sets_equal(value_batch[1], search::knn(space, 8, 2)));

	const std::vector<int> records = {0, 4, 8, 12};
	const auto records_batch = search::knn_batch(records, AbsoluteDistance{}, queries, 2);
	assert(records_batch.size() == 2);
	assert(neighbor_sets_equal(records_batch[0], search::find_neighbors(records, AbsoluteDistance{}, 0, 2)));
	assert(neighbor_sets_equal(records_batch[1], search::find_neighbors(records, AbsoluteDistance{}, 8, 2)));
	assert(records_batch[0].representation == "records");
	assert(records_batch[0].provenance.route == "source_metric");
	assert(records_batch[0].provenance.route_kind == mtrc::search_route_kind::source_metric);
	assert(records_batch[0].provenance.domain_kind == mtrc::result_domain_kind::records);

	const mtrc::space::storage::LiveDistances<decltype(space)> provider(space);
	const auto provider_batch = search::knn_batch(provider, ids, 1);
	assert(provider_batch.size() == 2);
	assert(neighbor_sets_equal(provider_batch[0], search::knn(provider, space.id(0), 1)));
	assert(neighbor_sets_equal(provider_batch[1], search::knn(provider, space.id(1), 1)));
	assert(provider_batch[0].representation == "pairwise_distances");
	assert(provider_batch[0].provenance.route == "pairwise_provider");
	assert(provider_batch[0].provenance.route_kind == mtrc::search_route_kind::pairwise_provider);
	assert(provider_batch[0].provenance.domain_kind == mtrc::result_domain_kind::pairwise_provider);

	// An empty query list yields an empty result list, not a crash.
	const auto empty_batch = search::knn_batch(space, std::vector<mtrc::RecordId>{}, 3);
	assert(empty_batch.empty());
}

void range_batch_matches_single_queries()
{
	namespace search = mtrc::stats::search;
	const auto space = mtrc::make_space(std::vector<int>{0, 4, 8, 12}, AbsoluteDistance{});

	const std::vector<mtrc::RecordId> ids = {space.id(1), space.id(2)};
	const auto id_batch = search::range_batch(space, ids, 4);
	assert(id_batch.size() == 2);
	assert(neighbor_sets_equal(id_batch[0], search::range(space, space.id(1), 4)));
	assert(neighbor_sets_equal(id_batch[1], search::range(space, space.id(2), 4)));

	const std::vector<int> queries = {0, 12};
	const auto value_batch = search::range_batch(space, queries, 4);
	assert(value_batch.size() == 2);
	assert(neighbor_sets_equal(value_batch[0], search::range(space, 0, 4)));
	assert(neighbor_sets_equal(value_batch[1], search::range(space, 12, 4)));

	const std::vector<int> records = {0, 4, 8, 12};
	const auto records_batch = search::range_batch(records, AbsoluteDistance{}, queries, 4);
	assert(records_batch.size() == 2);
	assert(neighbor_sets_equal(records_batch[0], search::range(records, AbsoluteDistance{}, 0, 4)));
	assert(neighbor_sets_equal(records_batch[1], search::range(records, AbsoluteDistance{}, 12, 4)));
	assert(records_batch[0].representation == "records");
	assert(records_batch[0].provenance.route == "source_metric");
	assert(records_batch[0].provenance.route_kind == mtrc::search_route_kind::source_metric);
	assert(records_batch[0].provenance.domain_kind == mtrc::result_domain_kind::records);

	const mtrc::space::storage::LiveDistances<decltype(space)> provider(space);
	const auto provider_batch = search::range_batch(provider, ids, 4);
	assert(provider_batch.size() == 2);
	assert(neighbor_sets_equal(provider_batch[0], search::range(provider, space.id(1), 4)));
	assert(neighbor_sets_equal(provider_batch[1], search::range(provider, space.id(2), 4)));
	assert(provider_batch[0].representation == "pairwise_distances");
	assert(provider_batch[0].provenance.route == "pairwise_provider");
	assert(provider_batch[0].provenance.route_kind == mtrc::search_route_kind::pairwise_provider);
	assert(provider_batch[0].provenance.domain_kind == mtrc::result_domain_kind::pairwise_provider);

	// Invalid radius is rejected up front for the whole batch.
	assert(throws_invalid_argument([&] { (void)search::range_batch(space, ids, -1); }));
	assert(throws_invalid_argument([&] { (void)search::range_batch(provider, std::vector<mtrc::RecordId>{}, -1); }));
	assert(throws_invalid_argument([&] { (void)search::range_batch(records, AbsoluteDistance{}, std::vector<int>{}, -1); }));
}

void regular_sample_carries_lineage()
{
	namespace sample = mtrc::stats::sample;
	const auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 10}, AbsoluteDistance{});

	const auto selection = sample::regular_sample(space, 2);
	assert(selection.algorithm == "regular_sample");
	assert(selection.representation == "metric_space");
	assert(selection.size() == 2);
	assert((selection.positions == std::vector<std::size_t>{0, 2}));
	assert(selection.record_ids[0] == space.id(0));
	assert(selection.record_ids[1] == space.id(2));
	assert(selection.record_count == 4);
	assert(selection.requested_count == 2);
	// nearest_sample_distances is one entry per RECORD (its distance to the nearest selected
	// sample), i.e. the coverage profile -- not one entry per selected sample.
	assert(selection.nearest_sample_distances.size() == 4);
	assert(selection.coverage_radius >= 0);

	// Determinism + boundary behavior.
	const auto again = sample::regular_sample(space, 2);
	assert(again.positions == selection.positions);
	assert(sample::regular_sample(space, 0).empty());
	assert(throws_invalid_argument([&] { (void)sample::regular_sample(space, 5); }));
}

void farthest_first_and_walk_are_deterministic()
{
	namespace sample = mtrc::stats::sample;
	const auto space = mtrc::make_space(std::vector<int>{0, 0, 0, 1, 100}, AbsoluteDistance{});

	const auto coverage = sample::farthest_first(space, 3);
	const auto coverage_again = sample::farthest_first(space, 3);
	assert((coverage.positions == std::vector<std::size_t>{0, 4, 3}));
	assert(coverage.positions == coverage_again.positions);

	const auto walk = sample::metric_walk(space, 4);
	const auto walk_again = sample::metric_walk(space, 4);
	assert(walk.positions == walk_again.positions);
	assert(walk.coverage_radius == 99);

	// Seed out of range is rejected on both deterministic samplers.
	assert(throws_out_of_range([&] { (void)sample::farthest_first(space, 1, sample::farthest_first_options(9)); }));
	assert(throws_out_of_range([&] { (void)sample::metric_walk(space, 1, sample::metric_walk_options(9)); }));
}

} // namespace

int main()
{
	knn_batch_matches_single_queries();
	range_batch_matches_single_queries();
	regular_sample_carries_lineage();
	farthest_first_and_walk_are_deterministic();
	return 0;
}
