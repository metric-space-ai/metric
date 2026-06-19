// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_HPP
#define _METRIC_OPERATORS_HPP

#include "space.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace metric::operators {

namespace detail {

template <typename Container> using record_type_t = typename std::decay<typename Container::value_type>::type;

template <typename Container, typename Metric>
using finite_space_t = ::metric::FiniteSpace<record_type_t<Container>, Metric>;

} // namespace detail

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
