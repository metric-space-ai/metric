// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_PROPERTIES_LOCAL_VOLUME_HPP
#define _METRIC_STATS_PROPERTIES_LOCAL_VOLUME_HPP

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/space/storage/implicit.hpp>

namespace mtrc::stats::properties {

template <typename Distance> struct LocalVolumeResult {
	using distance_type = Distance;

	std::vector<std::size_t> counts;
	std::vector<double> densities;
	std::size_t record_count{};
	Distance radius{};
	std::size_t minimum_count{};
	std::size_t maximum_count{};
	double average_count{};
	double minimum_density{};
	double maximum_density{};
	double average_density{};
	bool exact{true};
	std::string algorithm{"local_volume"};
	std::string representation;

	auto empty() const -> bool { return counts.empty(); }
	auto size() const -> std::size_t { return counts.size(); }
};

namespace local_volume_detail {

template <typename Value> auto finite_scalar(Value value) -> bool
{
	if constexpr (std::is_floating_point<Value>::value) {
		return std::isfinite(static_cast<double>(value));
	} else {
		(void)value;
		return true;
	}
}

template <typename Distance>
auto make_local_volume_result(std::vector<std::size_t> counts, std::vector<double> densities,
							  std::size_t record_count, Distance radius, std::string representation)
	-> LocalVolumeResult<Distance>
{
	LocalVolumeResult<Distance> result;
	result.counts = std::move(counts);
	result.densities = std::move(densities);
	result.record_count = record_count;
	result.radius = radius;
	result.representation = std::move(representation);

	if (result.counts.empty()) {
		return result;
	}

	result.minimum_count = result.counts.front();
	result.maximum_count = result.counts.front();
	result.minimum_density = result.densities.front();
	result.maximum_density = result.densities.front();

	double count_sum = 0.0;
	double density_sum = 0.0;
	for (std::size_t index = 0; index < result.counts.size(); ++index) {
		const auto count = result.counts[index];
		const auto density = result.densities[index];
		if (count < result.minimum_count) {
			result.minimum_count = count;
		}
		if (result.maximum_count < count) {
			result.maximum_count = count;
		}
		if (density < result.minimum_density) {
			result.minimum_density = density;
		}
		if (result.maximum_density < density) {
			result.maximum_density = density;
		}
		count_sum += static_cast<double>(count);
		density_sum += density;
	}

	result.average_count = count_sum / static_cast<double>(result.counts.size());
	result.average_density = density_sum / static_cast<double>(result.densities.size());
	return result;
}

} // namespace local_volume_detail

// Local volume counts how many records lie in each closed metric ball B(x, r),
// including x itself. Density is that count normalized by the finite record count.
template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto local_volume(const Provider &provider, Radius radius) -> LocalVolumeResult<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;
	using comparison_type = typename std::common_type<distance_type, Radius>::type;

	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
	if (!local_volume_detail::finite_scalar(radius)) {
		throw std::invalid_argument("radius must be finite");
	}

	const auto record_count = provider.record_count();
	std::vector<std::size_t> counts(record_count, 0);
	std::vector<double> densities(record_count, 0.0);
	if (record_count == 0) {
		return local_volume_detail::make_local_volume_result(
			std::move(counts), std::move(densities), record_count, static_cast<distance_type>(radius),
			"pairwise_distances");
	}

	const auto threshold = static_cast<comparison_type>(radius);
	for (std::size_t source = 0; source < record_count; ++source) {
		const auto source_id = provider.id(source);
		for (std::size_t target = 0; target < record_count; ++target) {
			const auto distance = provider.distance(source_id, provider.id(target));
			if (!local_volume_detail::finite_scalar(distance)) {
				throw std::invalid_argument("distance values must be finite");
			}
			if (static_cast<comparison_type>(distance) <= threshold) {
				++counts[source];
			}
		}
		densities[source] = static_cast<double>(counts[source]) / static_cast<double>(record_count);
	}

	return local_volume_detail::make_local_volume_result(
		std::move(counts), std::move(densities), record_count, static_cast<distance_type>(radius),
		"pairwise_distances");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto local_volume(const Space &space, Radius radius) -> LocalVolumeResult<typename Space::distance_type>
{
	space::storage::LiveDistances<Space> provider(space);
	auto result = local_volume(provider, radius);
	result.representation = "metric_space";
	return result;
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto density(const Provider &provider, Radius radius) -> LocalVolumeResult<typename Provider::distance_type>
{
	auto result = local_volume(provider, radius);
	result.algorithm = "density";
	return result;
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto density(const Space &space, Radius radius) -> LocalVolumeResult<typename Space::distance_type>
{
	auto result = local_volume(space, radius);
	result.algorithm = "density";
	return result;
}

} // namespace mtrc::stats::properties

#endif
