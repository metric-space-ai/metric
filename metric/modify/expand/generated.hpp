// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MODIFY_EXPAND_GENERATED_HPP
#define _METRIC_MODIFY_EXPAND_GENERATED_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>

namespace mtrc::modify::expand {

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto append_generated_records(const Space &space, std::vector<typename Space::record_type> generated_records,
							  std::vector<std::vector<RecordId>> generated_source_records)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;

	if (generated_records.empty()) {
		throw std::invalid_argument("expand requires at least one generated record");
	}
	if (generated_records.size() != generated_source_records.size()) {
		throw std::invalid_argument("generated record lineage count does not match generated record count");
	}

	auto target_records = space.records();
	target_records.reserve(space.size() + generated_records.size());
	target_records.insert(target_records.end(), generated_records.begin(), generated_records.end());

	std::vector<std::vector<RecordId>> source_records;
	source_records.reserve(target_records.size());
	std::vector<RecordId> representative_records;
	representative_records.reserve(target_records.size());

	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto id = space.id(index);
		source_records.push_back({id});
		representative_records.push_back(id);
	}

	for (auto &lineage : generated_source_records) {
		if (lineage.empty()) {
			throw std::invalid_argument("generated record lineage must name at least one source record");
		}
		for (const auto id : lineage) {
			if (!space.contains(id)) {
				throw std::invalid_argument("generated record lineage references a record outside the source space");
			}
		}
		representative_records.push_back(lineage.front());
		source_records.push_back(std::move(lineage));
	}

	target_space_type expanded_space(std::move(target_records), space.metric());
	return core::make_mapping_result(
		std::move(expanded_space), std::move(source_records), std::move(representative_records), space.size(), false,
		"generated_record_expansion", "caller_generated_records", "metric_space",
		core::metric_traits<typename Space::metric_type>::law, false,
		"record-set expansion with caller-generated records; original records are preserved unchanged, generated "
		"records must declare source lineage, and the expanded space uses the source metric over the enlarged record "
		"domain");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto expand(const Space &space, std::vector<typename Space::record_type> generated_records,
			std::vector<std::vector<RecordId>> generated_source_records)
	-> decltype(append_generated_records(space, std::move(generated_records), std::move(generated_source_records)))
{
	return append_generated_records(space, std::move(generated_records), std::move(generated_source_records));
}

} // namespace mtrc::modify::expand

namespace mtrc {
using modify::expand::append_generated_records;
using modify::expand::expand;
} // namespace mtrc

#endif
