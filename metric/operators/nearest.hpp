// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_NEAREST_HPP
#define _METRIC_OPERATORS_NEAREST_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/record_id.hpp"
#include "../core/result.hpp"

namespace metric::operators {

namespace engine_detail {

template <typename Distance>
auto sort_neighbors(std::vector<Neighbor<Distance>> &neighbors) -> void
{
	std::sort(neighbors.begin(), neighbors.end(), [](const auto &lhs, const auto &rhs) {
		if (lhs.distance == rhs.distance) {
			return lhs.id < rhs.id;
		}
		return lhs.distance < rhs.distance;
	});
}

template <typename Distance>
auto take_count(std::vector<Neighbor<Distance>> neighbors, std::size_t k, std::size_t record_count,
				const char *representation) -> NeighborSet<Distance>
{
	sort_neighbors(neighbors);
	if (neighbors.size() > k) {
		neighbors.resize(k);
	}

	NeighborSet<Distance> result;
	result.neighbors = std::move(neighbors);
	result.record_count = record_count;
	result.requested_count = k;
	result.exact = true;
	result.operator_name = "knn";
	result.representation = representation;
	return result;
}

template <typename Distance>
auto take_radius(std::vector<Neighbor<Distance>> neighbors, std::size_t record_count,
				 const char *representation) -> NeighborSet<Distance>
{
	sort_neighbors(neighbors);

	NeighborSet<Distance> result;
	result.neighbors = std::move(neighbors);
	result.record_count = record_count;
	result.requested_count = result.neighbors.size();
	result.exact = true;
	result.operator_name = "range";
	result.representation = representation;
	return result;
}

template <typename Radius>
auto validate_radius(Radius radius) -> void
{
	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
}

} // namespace engine_detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto knn(const Space &space, const typename Space::record_type &query, std::size_t k)
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	std::vector<Neighbor<distance_type>> candidates;
	candidates.reserve(space.size());
	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto id = space.id(index);
		candidates.push_back(Neighbor<distance_type>{id, space.metric()(query, space.record(id))});
	}

	return engine_detail::take_count(std::move(candidates), k, space.size(), "metric_space");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto knn(const Space &space, RecordId query_id, std::size_t k) -> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	(void)space.record(query_id);
	std::vector<Neighbor<distance_type>> candidates;
	candidates.reserve(space.size() > 0 ? space.size() - 1 : 0);
	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto id = space.id(index);
		if (id == query_id) {
			continue;
		}
		candidates.push_back(Neighbor<distance_type>{id, space.distance(query_id, id)});
	}

	return engine_detail::take_count(std::move(candidates), k, space.size(), "metric_space");
}

template <typename Provider, typename std::enable_if<DistanceProvider_v<Provider>, int>::type = 0>
auto knn(const Provider &provider, RecordId query_id, std::size_t k) -> NeighborSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (!provider.contains(query_id)) {
		throw std::out_of_range("query record id is outside the distance provider");
	}

	std::vector<Neighbor<distance_type>> candidates;
	candidates.reserve(provider.record_count() > 0 ? provider.record_count() - 1 : 0);
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto id = provider.id(index);
		if (id == query_id) {
			continue;
		}
		candidates.push_back(Neighbor<distance_type>{id, provider.distance(query_id, id)});
	}

	return engine_detail::take_count(std::move(candidates), k, provider.record_count(), "distance_provider");
}

template <typename Index, typename std::enable_if<NeighborSearchIndex_v<Index>, int>::type = 0>
auto knn(const Index &index, const typename Index::record_type &query, std::size_t k)
	-> NeighborSet<typename Index::distance_type>
{
	NeighborSet<typename Index::distance_type> result;
	result.neighbors = index.knn(query, k);
	result.record_count = index.record_count();
	result.requested_count = k;
	result.exact = true;
	result.operator_name = "knn";
	result.representation = "neighbor_index";
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius)
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	engine_detail::validate_radius(radius);
	const auto threshold = static_cast<comparison_type>(radius);

	std::vector<Neighbor<distance_type>> candidates;
	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto id = space.id(index);
		const auto distance = space.metric()(query, space.record(id));
		if (static_cast<comparison_type>(distance) <= threshold) {
			candidates.push_back(Neighbor<distance_type>{id, distance});
		}
	}

	return engine_detail::take_radius(std::move(candidates), space.size(), "metric_space");
}

template <typename Provider, typename Radius, typename std::enable_if<DistanceProvider_v<Provider>, int>::type = 0>
auto range(const Provider &provider, RecordId query_id, Radius radius) -> NeighborSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	engine_detail::validate_radius(radius);
	if (!provider.contains(query_id)) {
		throw std::out_of_range("query record id is outside the distance provider");
	}
	const auto threshold = static_cast<comparison_type>(radius);

	std::vector<Neighbor<distance_type>> candidates;
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto id = provider.id(index);
		if (id == query_id) {
			continue;
		}
		const auto distance = provider.distance(query_id, id);
		if (static_cast<comparison_type>(distance) <= threshold) {
			candidates.push_back(Neighbor<distance_type>{id, distance});
		}
	}

	return engine_detail::take_radius(std::move(candidates), provider.record_count(), "distance_provider");
}

} // namespace metric::operators

#endif
