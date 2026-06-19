// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_HPP
#define _METRIC_OPERATORS_HPP

#include "space.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <tuple>
#include <utility>
#include <vector>

namespace metric::operators {

namespace detail {

template <typename Container> using record_type_t = typename std::decay<typename Container::value_type>::type;

template <typename Container, typename Metric>
using finite_space_t = ::metric::FiniteSpace<record_type_t<Container>, Metric>;

} // namespace detail

template <typename Distance, typename RadiusValue = Distance> struct GraphConstructionMetadata {
	std::string strategy;
	std::size_t record_count{};
	std::size_t edge_count{};
	bool directed{true};
	bool self_loops{false};
	bool exact{true};
	std::optional<std::size_t> k;
	std::optional<RadiusValue> radius;
	std::string edge_payload;
	std::string weighting;
	std::string symmetrization;
	std::string normalization;
	std::string tie_break;
};

template <typename Distance, typename RadiusValue = Distance> struct GraphConstructionResult {
	using distance_type = Distance;
	using radius_type = RadiusValue;
	using edge_type = std::tuple<std::size_t, std::size_t, Distance>;

	std::vector<edge_type> edges;
	GraphConstructionMetadata<Distance, RadiusValue> metadata;
};

template <typename Container, typename Metric>
auto pairwise_distance_matrix(const Container &records, Metric distance)
	-> std::vector<std::vector<typename detail::finite_space_t<Container, Metric>::distance_type>>
{
	return ::metric::Space::from_records(records, std::move(distance)).pairwise_distances();
}

template <typename Container, typename Metric>
auto nearest_neighbors(const Container &records, Metric distance, const detail::record_type_t<Container> &query,
					   unsigned k = 1)
{
	return ::metric::Space::from_records(records, std::move(distance)).neighbors(query, k);
}

template <typename Container, typename Metric>
auto range_neighbors(const Container &records, Metric distance, const detail::record_type_t<Container> &query,
					 typename detail::finite_space_t<Container, Metric>::distance_type radius)
{
	return ::metric::Space::from_records(records, std::move(distance)).within_radius(query, radius);
}

template <typename Container, typename Metric>
auto exact_knn_graph(const Container &records, Metric distance, std::size_t k)
	-> GraphConstructionResult<typename detail::finite_space_t<Container, Metric>::distance_type>
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;

	GraphConstructionResult<distance_type> result;
	result.metadata.strategy = "exact_knn";
	result.metadata.record_count = records.size();
	result.metadata.directed = true;
	result.metadata.self_loops = false;
	result.metadata.exact = true;
	result.metadata.k = k;
	result.metadata.edge_payload = "metric_distance";
	result.metadata.weighting = "none";
	result.metadata.symmetrization = "none";
	result.metadata.normalization = "none";
	result.metadata.tie_break = "distance_then_target_index";

	if (k == 0) {
		return result;
	}

	const auto max_neighbors = records.empty() ? std::size_t{0} : records.size() - 1;
	if (k > max_neighbors) {
		throw std::invalid_argument("k cannot exceed the number of non-self neighbors");
	}

	const auto space = ::metric::Space::from_records(records, std::move(distance));
	result.edges.reserve(records.size() * k);

	for (std::size_t source_index = 0; source_index < records.size(); ++source_index) {
		std::vector<std::pair<distance_type, std::size_t>> candidates;
		candidates.reserve(records.size() - 1);

		for (std::size_t target_index = 0; target_index < records.size(); ++target_index) {
			if (source_index == target_index) {
				continue;
			}
			candidates.emplace_back(space.distance(source_index, target_index), target_index);
		}

		std::sort(candidates.begin(), candidates.end(),
				  [](const auto &lhs, const auto &rhs) {
					  if (lhs.first < rhs.first) {
						  return true;
					  }
					  if (rhs.first < lhs.first) {
						  return false;
					  }
					  return lhs.second < rhs.second;
				  });

		for (std::size_t neighbor_index = 0; neighbor_index < k; ++neighbor_index) {
			result.edges.emplace_back(source_index, candidates[neighbor_index].second, candidates[neighbor_index].first);
		}
	}

	result.metadata.edge_count = result.edges.size();
	return result;
}

template <typename Container, typename Metric>
auto exact_knn_graph_edges(const Container &records, Metric distance, std::size_t k)
	-> std::vector<
		std::tuple<std::size_t, std::size_t, typename detail::finite_space_t<Container, Metric>::distance_type>>
{
	return exact_knn_graph(records, std::move(distance), k).edges;
}

template <typename Container, typename Metric, typename Radius>
auto exact_radius_graph(const Container &records, Metric distance, Radius radius)
	-> GraphConstructionResult<typename detail::finite_space_t<Container, Metric>::distance_type,
							   typename std::common_type<
								   typename detail::finite_space_t<Container, Metric>::distance_type, Radius>::type>
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}

	GraphConstructionResult<distance_type, comparison_type> result;
	result.metadata.strategy = "exact_radius";
	result.metadata.record_count = records.size();
	result.metadata.directed = true;
	result.metadata.self_loops = false;
	result.metadata.exact = true;
	result.metadata.radius = static_cast<comparison_type>(radius);
	result.metadata.edge_payload = "metric_distance";
	result.metadata.weighting = "none";
	result.metadata.symmetrization = "none";
	result.metadata.normalization = "none";
	result.metadata.tie_break = "source_then_target_index";

	const auto threshold = static_cast<comparison_type>(radius);
	const auto space = ::metric::Space::from_records(records, std::move(distance));

	for (std::size_t source_index = 0; source_index < records.size(); ++source_index) {
		for (std::size_t target_index = 0; target_index < records.size(); ++target_index) {
			if (source_index == target_index) {
				continue;
			}
			const auto edge_distance = space.distance(source_index, target_index);
			if (static_cast<comparison_type>(edge_distance) <= threshold) {
				result.edges.emplace_back(source_index, target_index, edge_distance);
			}
		}
	}

	result.metadata.edge_count = result.edges.size();
	return result;
}

template <typename Container, typename Metric, typename Radius>
auto exact_radius_graph_edges(const Container &records, Metric distance, Radius radius)
	-> std::vector<
		std::tuple<std::size_t, std::size_t, typename detail::finite_space_t<Container, Metric>::distance_type>>
{
	return exact_radius_graph(records, std::move(distance), radius).edges;
}

template <typename Distance, typename RadiusValue>
auto symmetrize_graph(const GraphConstructionResult<Distance, RadiusValue> &graph,
					  const std::string &policy = "union",
					  const std::string &weighting = "minimum_distance")
	-> GraphConstructionResult<Distance, RadiusValue>
{
	if (policy != "union" && policy != "mutual") {
		throw std::invalid_argument("symmetrization policy must be 'union' or 'mutual'");
	}
	if (weighting != "minimum_distance" && weighting != "maximum_distance") {
		throw std::invalid_argument("weighting policy must be 'minimum_distance' or 'maximum_distance'");
	}

	struct Accumulator {
		std::optional<Distance> forward;
		std::optional<Distance> reverse;
	};

	auto merge_weight = [&weighting](Distance lhs, Distance rhs) {
		if (weighting == "minimum_distance") {
			return std::min(lhs, rhs);
		}
		return std::max(lhs, rhs);
	};

	auto assign_weight = [&merge_weight](std::optional<Distance> &slot, Distance value) {
		if (slot.has_value()) {
			slot = merge_weight(slot.value(), value);
		} else {
			slot = value;
		}
	};

	std::map<std::pair<std::size_t, std::size_t>, Accumulator> edge_accumulators;
	for (const auto &edge : graph.edges) {
		const auto source_index = std::get<0>(edge);
		const auto target_index = std::get<1>(edge);
		const auto distance = std::get<2>(edge);

		if (source_index == target_index) {
			continue;
		}

		const auto lower_index = std::min(source_index, target_index);
		const auto upper_index = std::max(source_index, target_index);
		auto &accumulator = edge_accumulators[{lower_index, upper_index}];
		if (source_index == lower_index) {
			assign_weight(accumulator.forward, distance);
		} else {
			assign_weight(accumulator.reverse, distance);
		}
	}

	GraphConstructionResult<Distance, RadiusValue> result;
	result.metadata = graph.metadata;
	result.metadata.directed = false;
	result.metadata.self_loops = false;
	result.metadata.symmetrization = policy;
	result.metadata.weighting = weighting;
	result.metadata.tie_break = "source_index_then_target_index";

	for (const auto &entry : edge_accumulators) {
		const auto &key = entry.first;
		const auto &accumulator = entry.second;
		const auto has_forward = accumulator.forward.has_value();
		const auto has_reverse = accumulator.reverse.has_value();

		if (policy == "mutual" && !(has_forward && has_reverse)) {
			continue;
		}

		if (has_forward && has_reverse) {
			result.edges.emplace_back(key.first, key.second,
									  merge_weight(accumulator.forward.value(), accumulator.reverse.value()));
		} else if (has_forward) {
			result.edges.emplace_back(key.first, key.second, accumulator.forward.value());
		} else {
			result.edges.emplace_back(key.first, key.second, accumulator.reverse.value());
		}
	}

	result.metadata.edge_count = result.edges.size();
	return result;
}

template <typename Container, typename Metric>
auto representative_indices(const Container &records, Metric distance, std::size_t k, std::size_t seed_index = 0)
	-> std::vector<std::size_t>
{
	if (k == 0) {
		return {};
	}
	if (records.empty()) {
		throw std::invalid_argument("cannot select representatives from an empty record set");
	}
	if (k > records.size()) {
		throw std::invalid_argument("k cannot exceed the number of records");
	}
	if (seed_index >= records.size()) {
		throw std::out_of_range("seed_index is outside the record set");
	}

	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;

	const auto space = ::metric::Space::from_records(records, std::move(distance));
	std::vector<std::size_t> selected = {seed_index};
	std::vector<bool> is_selected(records.size(), false);
	is_selected[seed_index] = true;

	std::vector<distance_type> nearest_selected_distances(records.size());
	for (std::size_t index = 0; index < records.size(); ++index) {
		nearest_selected_distances[index] = space.distance(index, seed_index);
	}

	while (selected.size() < k) {
		std::size_t next_index = records.size();
		distance_type next_distance{};
		bool has_next = false;

		for (std::size_t index = 0; index < nearest_selected_distances.size(); ++index) {
			if (is_selected[index]) {
				continue;
			}
			if (!has_next || nearest_selected_distances[index] > next_distance) {
				next_index = index;
				next_distance = nearest_selected_distances[index];
				has_next = true;
			}
		}

		if (!has_next) {
			throw std::logic_error("failed to select the next representative");
		}

		selected.push_back(next_index);
		is_selected[next_index] = true;
		for (std::size_t index = 0; index < nearest_selected_distances.size(); ++index) {
			nearest_selected_distances[index] =
				std::min(nearest_selected_distances[index], space.distance(index, next_index));
		}
	}

	return selected;
}

template <typename Container, typename Metric>
auto representatives(const Container &records, Metric distance, std::size_t k, std::size_t seed_index = 0)
	-> std::vector<detail::record_type_t<Container>>
{
	const auto selected = representative_indices(records, std::move(distance), k, seed_index);
	std::vector<detail::record_type_t<Container>> result;
	result.reserve(selected.size());
	for (const auto index : selected) {
		result.push_back(records[index]);
	}
	return result;
}

template <typename Container, typename Metric>
auto medoid_index(const Container &records, Metric distance) -> std::size_t
{
	if (records.empty()) {
		throw std::invalid_argument("cannot select a medoid from an empty record set");
	}

	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;

	const auto space = ::metric::Space::from_records(records, std::move(distance));
	std::size_t best_index = 0;
	distance_type best_total_distance{};
	bool has_best = false;

	for (std::size_t candidate_index = 0; candidate_index < records.size(); ++candidate_index) {
		distance_type total_distance{};
		for (std::size_t other_index = 0; other_index < records.size(); ++other_index) {
			total_distance += space.distance(candidate_index, other_index);
		}

		if (!has_best || total_distance < best_total_distance) {
			best_index = candidate_index;
			best_total_distance = total_distance;
			has_best = true;
		}
	}

	return best_index;
}

template <typename Container, typename Metric>
auto medoid(const Container &records, Metric distance) -> detail::record_type_t<Container>
{
	return records[medoid_index(records, std::move(distance))];
}

template <typename Container, typename Metric, typename MinimumDistance>
auto separated_representative_indices(const Container &records, Metric distance, MinimumDistance minimum_distance)
	-> std::vector<std::size_t>
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;
	using comparison_type = typename std::common_type<distance_type, MinimumDistance>::type;

	if (minimum_distance < MinimumDistance{}) {
		throw std::invalid_argument("minimum_distance must be non-negative");
	}
	if (records.empty()) {
		return {};
	}

	const auto threshold = static_cast<comparison_type>(minimum_distance);
	const auto space = ::metric::Space::from_records(records, std::move(distance));
	std::vector<std::size_t> selected;

	for (std::size_t candidate_index = 0; candidate_index < records.size(); ++candidate_index) {
		bool is_separated = true;
		for (const auto selected_index : selected) {
			if (static_cast<comparison_type>(space.distance(candidate_index, selected_index)) < threshold) {
				is_separated = false;
				break;
			}
		}
		if (is_separated) {
			selected.push_back(candidate_index);
		}
	}

	return selected;
}

template <typename Container, typename Metric, typename MinimumDistance>
auto separated_representatives(const Container &records, Metric distance, MinimumDistance minimum_distance)
	-> std::vector<detail::record_type_t<Container>>
{
	const auto selected = separated_representative_indices(records, std::move(distance), minimum_distance);
	std::vector<detail::record_type_t<Container>> result;
	result.reserve(selected.size());
	for (const auto index : selected) {
		result.push_back(records[index]);
	}
	return result;
}

template <typename Container, typename Metric, typename Radius>
auto coverage_representative_indices(const Container &records, Metric distance, Radius radius)
	-> std::vector<std::size_t>
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;

	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
	if (records.empty()) {
		return {};
	}

	const auto cover_radius = static_cast<distance_type>(radius);
	const auto space = ::metric::Space::from_records(records, std::move(distance));
	std::vector<std::size_t> selected;
	std::vector<bool> covered(records.size(), false);
	std::size_t covered_count = 0;

	while (covered_count < records.size()) {
		auto seed_index = records.size();
		for (std::size_t index = 0; index < covered.size(); ++index) {
			if (!covered[index]) {
				seed_index = index;
				break;
			}
		}
		if (seed_index == records.size()) {
			throw std::logic_error("failed to select the next coverage representative");
		}

		selected.push_back(seed_index);
		for (std::size_t index = 0; index < covered.size(); ++index) {
			if (!covered[index] && space.distance(seed_index, index) <= cover_radius) {
				covered[index] = true;
				++covered_count;
			}
		}
	}

	return selected;
}

template <typename Container, typename Metric, typename Radius>
auto coverage_representatives(const Container &records, Metric distance, Radius radius)
	-> std::vector<detail::record_type_t<Container>>
{
	const auto selected = coverage_representative_indices(records, std::move(distance), radius);
	std::vector<detail::record_type_t<Container>> result;
	result.reserve(selected.size());
	for (const auto index : selected) {
		result.push_back(records[index]);
	}
	return result;
}

template <typename Container, typename Metric>
auto intrinsic_dimension(const Container &records, Metric distance) -> double
{
	using distance_type = typename detail::finite_space_t<Container, Metric>::distance_type;

	const auto distances = pairwise_distance_matrix(records, std::move(distance));

	double maximum_dimension = 0.0;
	for (const auto &row : distances) {
		for (const auto &radius : row) {
			if (radius <= distance_type{}) {
				continue;
			}

			const auto outer_radius = radius + radius;
			const auto inner_count = static_cast<double>(
				std::count_if(row.begin(), row.end(), [&](const auto &value) { return value <= radius; }));
			const auto outer_count = static_cast<double>(
				std::count_if(row.begin(), row.end(), [&](const auto &value) { return value <= outer_radius; }));

			if (inner_count > 0.0 && outer_count >= inner_count) {
				maximum_dimension = std::max(maximum_dimension, std::log(outer_count / inner_count) / std::log(2.0));
			}
		}
	}

	return maximum_dimension;
}

} // namespace metric::operators

#endif
