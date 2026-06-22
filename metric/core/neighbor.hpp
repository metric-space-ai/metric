// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_NEIGHBOR_HPP
#define _METRIC_CORE_NEIGHBOR_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/record/id.hpp>

namespace mtrc::core {

template <typename Distance> struct Neighbor {
	using distance_type = Distance;

	RecordId id;
	Distance distance{};
};

// A metric is required to return finite, non-negative distances, so a non-finite
// (NaN) distance only arises from a degenerate/unsupported metric evaluation. NaN
// breaks the strict-weak-ordering required by std::sort (NaN compares unordered
// against every value, so the equivalence relation is not transitive). This helper
// lets the neighbor comparator treat NaN as a well-defined "sorts last" bucket
// instead of invoking undefined behavior. Integral distances can never be NaN.
template <typename Distance> constexpr auto neighbor_distance_is_nan(const Distance &distance) -> bool
{
	if constexpr (std::is_floating_point<Distance>::value) {
		return std::isnan(distance);
	} else {
		(void)distance;
		return false;
	}
}

template <typename Distance, typename IdAt, typename DistanceAt, typename Include>
auto neighbor_candidates_if(std::size_t record_count, IdAt id_at, DistanceAt distance_at, Include include)
	-> std::vector<Neighbor<Distance>>
{
	std::vector<Neighbor<Distance>> candidates;
	candidates.reserve(record_count);
	for (std::size_t position = 0; position < record_count; ++position) {
		const auto id = id_at(position);
		if (include(id, position)) {
			candidates.push_back(Neighbor<Distance>{id, static_cast<Distance>(distance_at(id, position))});
		}
	}
	return candidates;
}

template <typename Distance, typename IdAt, typename DistanceAt>
auto neighbor_candidates(std::size_t record_count, IdAt id_at, DistanceAt distance_at) -> std::vector<Neighbor<Distance>>
{
	return neighbor_candidates_if<Distance>(record_count, id_at, distance_at,
											[](RecordId, std::size_t) { return true; });
}

template <typename Distance, typename Radius, typename IdAt, typename DistanceAt, typename Include>
auto neighbor_candidates_within_if(std::size_t record_count, IdAt id_at, DistanceAt distance_at, Radius radius,
								   Include include) -> std::vector<Neighbor<Distance>>
{
	using comparison_type = typename std::common_type<Distance, Radius>::type;

	const auto threshold = static_cast<comparison_type>(radius);
	std::vector<Neighbor<Distance>> candidates;
	candidates.reserve(record_count);
	for (std::size_t position = 0; position < record_count; ++position) {
		const auto id = id_at(position);
		if (!include(id, position)) {
			continue;
		}
		const auto distance = static_cast<Distance>(distance_at(id, position));
		if (static_cast<comparison_type>(distance) <= threshold) {
			candidates.push_back(Neighbor<Distance>{id, distance});
		}
	}
	return candidates;
}

template <typename Distance, typename Radius, typename IdAt, typename DistanceAt>
auto neighbor_candidates_within(std::size_t record_count, IdAt id_at, DistanceAt distance_at, Radius radius)
	-> std::vector<Neighbor<Distance>>
{
	return neighbor_candidates_within_if<Distance>(record_count, id_at, distance_at, radius,
												   [](RecordId, std::size_t) { return true; });
}

// Orders neighbors by distance ascending, breaking ties deterministically by RecordId
// ascending. The comparator is a true strict weak ordering even when a degenerate metric
// yields NaN distances: any NaN sorts after every finite/infinite distance, and NaN-vs-NaN
// ties fall back to RecordId order. This keeps knn/range output deterministic and avoids the
// std::sort undefined behavior a raw `<` comparator would trigger on NaN.
template <typename Distance> auto sort_neighbors(std::vector<Neighbor<Distance>> &neighbors) -> void
{
	std::sort(neighbors.begin(), neighbors.end(), [](const auto &lhs, const auto &rhs) {
		const bool lhs_nan = neighbor_distance_is_nan(lhs.distance);
		const bool rhs_nan = neighbor_distance_is_nan(rhs.distance);
		if (lhs_nan != rhs_nan) {
			return !lhs_nan; // finite/infinite distances order before NaN
		}
		if (!lhs_nan) {
			if (lhs.distance < rhs.distance) {
				return true;
			}
			if (rhs.distance < lhs.distance) {
				return false;
			}
		}
		return lhs.id < rhs.id;
	});
}

template <typename Distance>
auto take_nearest_neighbors(std::vector<Neighbor<Distance>> neighbors, std::size_t count)
	-> std::vector<Neighbor<Distance>>
{
	sort_neighbors(neighbors);
	if (neighbors.size() > count) {
		neighbors.resize(count);
	}
	return neighbors;
}

template <typename Distance>
auto take_neighbors_excluding_id(const std::vector<Neighbor<Distance>> &neighbors, RecordId excluded_id,
								 std::size_t count) -> std::vector<Neighbor<Distance>>
{
	std::vector<Neighbor<Distance>> selected;
	if (count == 0) {
		return selected;
	}
	selected.reserve(std::min(count, neighbors.size()));
	for (const auto &neighbor : neighbors) {
		if (neighbor.id == excluded_id) {
			continue;
		}
		selected.push_back(neighbor);
		if (selected.size() == count) {
			break;
		}
	}
	return selected;
}

template <typename Distance>
auto neighbor_ids(const std::vector<Neighbor<Distance>> &neighbors, std::size_t count) -> std::vector<RecordId>
{
	const auto selected_count = std::min(count, neighbors.size());
	std::vector<RecordId> ids;
	ids.reserve(selected_count);
	for (std::size_t index = 0; index < selected_count; ++index) {
		ids.push_back(neighbors[index].id);
	}
	return ids;
}

template <typename Distance> auto neighbor_ids(const std::vector<Neighbor<Distance>> &neighbors) -> std::vector<RecordId>
{
	return neighbor_ids(neighbors, neighbors.size());
}

template <typename Distance, typename PositionOf>
auto source_ids_for_neighbors(const std::vector<RecordId> &source_ids, const std::vector<Neighbor<Distance>> &neighbors,
							  PositionOf position_of, const char *invalid_position_message) -> std::vector<RecordId>
{
	std::vector<RecordId> ids;
	ids.reserve(neighbors.size());
	for (const auto &neighbor : neighbors) {
		const auto position = position_of(neighbor.id);
		if (position >= source_ids.size()) {
			throw std::invalid_argument(invalid_position_message);
		}
		ids.push_back(source_ids[position]);
	}
	return ids;
}

template <typename Distance>
auto take_nearest_neighbor_ids(std::vector<Neighbor<Distance>> neighbors, std::size_t count) -> std::vector<RecordId>
{
	const auto nearest = take_nearest_neighbors(std::move(neighbors), count);
	return neighbor_ids(nearest);
}

template <typename Distance>
auto neighbor_id_position(const std::vector<Neighbor<Distance>> &neighbors, RecordId id) -> std::size_t
{
	for (std::size_t index = 0; index < neighbors.size(); ++index) {
		if (neighbors[index].id == id) {
			return index;
		}
	}
	return neighbors.size();
}

template <typename Distance>
auto contains_neighbor_id(const std::vector<Neighbor<Distance>> &neighbors, RecordId id) -> bool
{
	return neighbor_id_position(neighbors, id) != neighbors.size();
}

template <typename Distance>
auto neighbor_rank(const std::vector<Neighbor<Distance>> &neighbors, RecordId id) -> std::size_t
{
	return neighbor_id_position(neighbors, id) + 1;
}

template <typename Distance>
auto neighbor_distance_or_throw(const std::vector<Neighbor<Distance>> &neighbors, RecordId id, const char *message)
	-> Distance
{
	const auto position = neighbor_id_position(neighbors, id);
	if (position == neighbors.size()) {
		throw std::invalid_argument(message);
	}
	return neighbors[position].distance;
}

template <typename ExpectedDistance, typename CandidateDistance>
auto neighbor_id_overlap_count(const std::vector<Neighbor<ExpectedDistance>> &expected,
							   const std::vector<Neighbor<CandidateDistance>> &candidates) -> std::size_t
{
	std::size_t matches = 0;
	for (const auto &candidate : candidates) {
		if (contains_neighbor_id(expected, candidate.id)) {
			++matches;
		}
	}
	return matches;
}

} // namespace mtrc::core

namespace mtrc {
template <typename Distance> using Neighbor = core::Neighbor<Distance>;
using core::contains_neighbor_id;
using core::neighbor_candidates;
using core::neighbor_candidates_if;
using core::neighbor_candidates_within;
using core::neighbor_candidates_within_if;
using core::neighbor_distance_or_throw;
using core::neighbor_id_overlap_count;
using core::neighbor_ids;
using core::neighbor_id_position;
using core::neighbor_rank;
using core::sort_neighbors;
using core::source_ids_for_neighbors;
using core::take_nearest_neighbor_ids;
using core::take_nearest_neighbors;
using core::take_neighbors_excluding_id;
} // namespace mtrc

#endif
