// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Search pipeline interoperability: source-space search and mapped-space search are
// both intentional routes. Results must expose which route was used instead of hiding
// a mapped Euclidean representative behind the original source metric.

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/core/result.hpp"
#include "metric/metric/catalog.hpp"
#include "metric/modify/map/clustered_space.hpp"
#include "metric/modify/map/map.hpp"
#include "metric/record/id.hpp"
#include "metric/space/storage/implicit.hpp"
#include "metric/stats/search/nearest.hpp"
#include "metric/stats/search/neighbors.hpp"

namespace {

struct IntDistance {
	auto operator()(int lhs, int rhs) const -> double
	{
		return static_cast<double>(lhs > rhs ? lhs - rhs : rhs - lhs);
	}
};

auto contains_id(const std::vector<mtrc::RecordId> &ids, mtrc::RecordId id) -> bool
{
	for (const auto candidate : ids) {
		if (candidate == id) {
			return true;
		}
	}
	return false;
}

template <typename Mapping, typename Query, typename = void>
struct mapped_knn_query_available : std::false_type {};

template <typename Mapping, typename Query>
struct mapped_knn_query_available<
	Mapping, Query,
	std::void_t<decltype(mtrc::find_neighbors(std::declval<const Mapping &>(), std::declval<const Query &>(),
											  std::size_t{}))>> : std::true_type {};

template <typename Mapping, typename Query, typename Radius, typename = void>
struct mapped_range_query_available : std::false_type {};

template <typename Mapping, typename Query, typename Radius>
struct mapped_range_query_available<
	Mapping, Query, Radius,
	std::void_t<decltype(mtrc::stats::search::range(std::declval<const Mapping &>(), std::declval<const Query &>(),
													std::declval<Radius>()))>> : std::true_type {};

} // namespace

int main()
{
	const auto source = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, IntDistance{});
	const auto source_query = source.id(1);

	const auto direct = mtrc::find_neighbors(source, source_query, 2);
	assert(direct.provenance.route == "source_metric");
	assert(direct.provenance.route_kind == mtrc::search_route_kind::source_metric);
	assert(direct.provenance.domain_kind == mtrc::result_domain_kind::metric_space);
	assert(direct.representation == "metric_space");
	assert(direct.exact);

	const auto exact_strategy_neighbors =
		mtrc::find_neighbors(source, source.record(source_query), 2, mtrc::stats::search::exact_scan{});
	assert(exact_strategy_neighbors.representation == "exact_scan_index");
	assert(exact_strategy_neighbors.provenance.route == "source_metric");
	assert(exact_strategy_neighbors.provenance.route_kind == mtrc::search_route_kind::source_metric);
	assert(exact_strategy_neighbors.provenance.domain_kind == mtrc::result_domain_kind::neighbor_index);

	const auto table_neighbors = mtrc::find_neighbors(source, source_query, 2, mtrc::stats::search::distance_table{});
	assert(table_neighbors.representation == "distance_table");
	assert(table_neighbors.provenance.route == "pairwise_provider");
	assert(table_neighbors.provenance.route_kind == mtrc::search_route_kind::pairwise_provider);
	assert(table_neighbors.provenance.domain_kind == mtrc::result_domain_kind::pairwise_provider);

	const auto graph_neighbors = mtrc::find_neighbors(source, source_query, 2, mtrc::stats::search::knn_graph{3});
	assert(graph_neighbors.representation == "knn_graph_index");
	assert(graph_neighbors.provenance.route == "source_metric");
	assert(graph_neighbors.provenance.route_kind == mtrc::search_route_kind::source_metric);
	assert(graph_neighbors.provenance.domain_kind == mtrc::result_domain_kind::neighbor_index);

	const mtrc::space::storage::LiveDistances<decltype(source)> provider(source);
	const auto provider_neighbors = mtrc::stats::search::knn(provider, source_query, 2);
	assert(provider_neighbors.provenance.route == "pairwise_provider");
	assert(provider_neighbors.provenance.route_kind == mtrc::search_route_kind::pairwise_provider);
	assert(provider_neighbors.provenance.domain_kind == mtrc::result_domain_kind::pairwise_provider);
	assert(provider_neighbors.representation == "pairwise_distances");

	const auto mapped = mtrc::map(source,
								  [](int value) {
									  return std::vector<double>{
										  static_cast<double>(value),
										  static_cast<double>(value * value),
									  };
								  },
								  mtrc::Euclidean<double>{});
	static_assert(!mapped_knn_query_available<decltype(mapped), int>::value,
				  "MappingResult search is lineage-only; raw source-record queries need a mapping-artifact API");
	static_assert(!mapped_range_query_available<decltype(mapped), int, double>::value,
				  "MappingResult range is lineage-only; raw source-record queries need a mapping-artifact API");

	const auto mapped_neighbors = mtrc::find_neighbors(mapped, source_query, 2);
	assert(mapped_neighbors.provenance.route == "mapped_space");
	assert(mapped_neighbors.provenance.route_kind == mtrc::search_route_kind::mapped_space);
	assert(mapped_neighbors.provenance.domain_kind == mtrc::result_domain_kind::mapped_space);
	assert(mapped_neighbors.provenance.mapping == "deterministic_transform");
	assert(mapped_neighbors.provenance.mapping_strategy == "deterministic_transform");
	assert(mapped_neighbors.provenance.has_query_source_id);
	assert(mapped_neighbors.provenance.query_source_id == source_query);
	assert(mapped_neighbors.provenance.representative_route);
	assert(!mapped_neighbors.exact);
	assert(mapped_neighbors.provenance.source_neighbor_ids.size() == mapped_neighbors.size());
	assert(mapped_neighbors.provenance.source_neighbor_lineage.size() == mapped_neighbors.size());
	assert(mapped_neighbors.provenance.source_neighbor_lineage[0].size() == 1);
	assert(contains_id(mapped_neighbors.provenance.source_neighbor_ids, source.id(0)));
	assert(contains_id(mapped_neighbors.provenance.source_neighbor_ids, source.id(2)));
	assert(contains_id(mapped_neighbors.provenance.representative_neighbor_ids, source.id(0)));
	assert(contains_id(mapped_neighbors.provenance.representative_neighbor_ids, source.id(2)));

	const std::vector<mtrc::RecordId> mapped_queries = {source.id(1), source.id(2)};
	const auto mapped_batch = mtrc::stats::search::knn_batch(mapped, mapped_queries, 2);
	assert(mapped_batch.size() == 2);
	assert(mapped_batch[0].provenance.route == "mapped_space");
	assert(mapped_batch[0].provenance.route_kind == mtrc::search_route_kind::mapped_space);
	assert(mapped_batch[0].provenance.domain_kind == mtrc::result_domain_kind::mapped_space);
	assert(mapped_batch[0].provenance.mapping == "deterministic_transform");
	assert(mapped_batch[0].provenance.source_neighbor_lineage.size() == mapped_batch[0].size());
	assert(mapped_batch[1].provenance.query_source_id == source.id(2));

	const auto clustered_source = mtrc::make_space(std::vector<int>{0, 1, 2, 10, 11}, IntDistance{});
	const auto clusters = mtrc::core::make_clustering_result<double>(
		std::vector<std::size_t>{0, 0, 0, 1, 1},
		std::vector<mtrc::RecordId>{clustered_source.id(1), clustered_source.id(3)}, std::vector<mtrc::RecordId>{},
		std::vector<mtrc::RecordId>{}, std::vector<std::size_t>{3, 2}, 1, true, "manual_clusters",
		"metric_space");
	const auto clustered = mtrc::modify::map::clustered_space(clustered_source, clusters);
	const auto clustered_neighbors = mtrc::find_neighbors(clustered, clustered_source.id(0), 1);
	assert(clustered_neighbors.provenance.route == "mapped_space");
	assert(clustered_neighbors.provenance.route_kind == mtrc::search_route_kind::mapped_space);
	assert(clustered_neighbors.provenance.domain_kind == mtrc::result_domain_kind::mapped_space);
	assert(clustered_neighbors.provenance.source_neighbor_lineage.size() == clustered_neighbors.size());
	assert(clustered_neighbors.provenance.source_neighbor_lineage[0].size() == 2);
	assert(contains_id(clustered_neighbors.provenance.source_neighbor_lineage[0], clustered_source.id(3)));
	assert(contains_id(clustered_neighbors.provenance.source_neighbor_lineage[0], clustered_source.id(4)));
	assert(clustered_neighbors.provenance.source_neighbor_ids.size() == 2);
	assert(clustered_neighbors.provenance.representative_neighbor_ids.size() == 1);
	assert(clustered_neighbors.provenance.representative_neighbor_ids[0] == clustered_source.id(3));

	const auto mapped_range = mtrc::stats::search::range(mapped, source_query, 4.0);
	assert(mapped_range.provenance.route == "mapped_space");
	assert(mapped_range.provenance.route_kind == mtrc::search_route_kind::mapped_space);
	assert(mapped_range.provenance.domain_kind == mtrc::result_domain_kind::mapped_space);
	assert(mapped_range.provenance.mapping == "deterministic_transform");
	assert(!mapped_range.exact);
	const auto mapped_range_batch = mtrc::stats::search::range_batch(mapped, mapped_queries, 4.0);
	assert(mapped_range_batch.size() == 2);
	assert(mapped_range_batch[0].provenance.route == "mapped_space");
	assert(mapped_range_batch[0].provenance.route_kind == mtrc::search_route_kind::mapped_space);
	assert(mapped_range_batch[0].provenance.domain_kind == mtrc::result_domain_kind::mapped_space);
	assert(mapped_range_batch[0].provenance.mapping_strategy == "deterministic_transform");

	auto malformed_mapping = mapped;
	malformed_mapping.validity.clear();
	bool rejected_malformed_knn_mapping = false;
	try {
		(void)mtrc::find_neighbors(malformed_mapping, source_query, 1);
	} catch (const std::invalid_argument &) {
		rejected_malformed_knn_mapping = true;
	}
	assert(rejected_malformed_knn_mapping);

	bool rejected_malformed_range_mapping = false;
	try {
		(void)mtrc::stats::search::range(malformed_mapping, source_query, 4.0);
	} catch (const std::invalid_argument &) {
		rejected_malformed_range_mapping = true;
	}
	assert(rejected_malformed_range_mapping);

	bool rejected_missing_source_id = false;
	try {
		(void)mtrc::find_neighbors(mapped, mtrc::RecordId::from_index(42), 1);
	} catch (const std::out_of_range &) {
		rejected_missing_source_id = true;
	}
	assert(rejected_missing_source_id);

	bool rejected_missing_range_source_id = false;
	try {
		(void)mtrc::stats::search::range(mapped, mtrc::RecordId::from_index(42), 4.0);
	} catch (const std::out_of_range &) {
		rejected_missing_range_source_id = true;
	}
	assert(rejected_missing_range_source_id);

	return 0;
}
