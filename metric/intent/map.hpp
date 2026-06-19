// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_MAP_HPP
#define _METRIC_INTENT_MAP_HPP

#include <type_traits>
#include <utility>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/metric_space.hpp"
#include "../core/record_id.hpp"
#include "../core/result.hpp"

namespace metric::intent {

template <typename Space, typename Transform, typename TargetMetric,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto map(const Space &space, Transform transform, TargetMetric metric)
	-> MappingResult<MetricSpace<typename std::decay<decltype(std::declval<Transform>()(
									 std::declval<const typename Space::record_type &>()))>::type,
								 typename std::decay<TargetMetric>::type>>
{
	using target_record_type = typename std::decay<decltype(
		std::declval<Transform>()(std::declval<const typename Space::record_type &>()))>::type;
	using target_metric_type = typename std::decay<TargetMetric>::type;
	using target_space_type = MetricSpace<target_record_type, target_metric_type>;
	using result_type = MappingResult<target_space_type>;

	std::vector<target_record_type> records;
	std::vector<std::vector<RecordId>> source_records;
	std::vector<RecordId> representative_records;
	records.reserve(space.size());
	source_records.reserve(space.size());
	representative_records.reserve(space.size());

	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto id = space.id(index);
		records.push_back(transform(space.record(id)));
		source_records.push_back({id});
		representative_records.push_back(id);
	}

	target_space_type derived_space(std::move(records), std::move(metric));
	return result_type{std::move(derived_space), std::move(source_records), std::move(representative_records),
					   space.size(), false, "deterministic_transform", "deterministic_transform", "metric_space"};
}

} // namespace metric::intent

namespace metric {
using intent::map;
} // namespace metric

#endif
