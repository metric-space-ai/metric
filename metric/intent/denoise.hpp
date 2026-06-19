// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_DENOISE_HPP
#define _METRIC_INTENT_DENOISE_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/metric_space.hpp"
#include "../core/record_id.hpp"
#include "../core/result.hpp"
#include "../operators/clustering.hpp"
#include "../strategies/clustering.hpp"

namespace metric::intent {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise(const Space &space, strategies::dbscan strategy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;
	using result_type = MappingResult<target_space_type>;

	const auto groups = operators::dbscan(space, strategy.radius, strategy.min_points);
	using clustering_type = typename std::decay<decltype(groups)>::type;
	std::vector<typename Space::record_type> records;
	std::vector<std::vector<RecordId>> source_records;
	std::vector<RecordId> representative_records;
	records.reserve(space.size());
	source_records.reserve(space.size());
	representative_records.reserve(space.size());

	for (std::size_t index = 0; index < groups.assignments.size(); ++index) {
		if (groups.assignments[index] == clustering_type::noise_label) {
			continue;
		}

		const auto id = space.id(index);
		records.push_back(space.record(id));
		source_records.push_back({id});
		representative_records.push_back(id);
	}

	target_space_type derived_space(std::move(records), space.metric());
	return result_type{std::move(derived_space), std::move(source_records), std::move(representative_records),
					   space.size(), false, "density_denoise", "dbscan_noise_filter", groups.representation};
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto denoise(const Space &space, double radius, std::size_t min_points) -> decltype(denoise(
	space, strategies::dbscan(radius, min_points)))
{
	return denoise(space, strategies::dbscan(radius, min_points));
}

} // namespace metric::intent

namespace metric {
using intent::denoise;
} // namespace metric

#endif
