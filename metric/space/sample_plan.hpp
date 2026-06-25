// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_SAMPLE_PLAN_HPP
#define _METRIC_SPACE_SAMPLE_PLAN_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include <metric/record/id.hpp>

namespace mtrc::space {

// Deterministic, reusable regular sample/candidate plan for approximate routes.
// The helper is intentionally passive: it owns only positions and metadata, so
// callers can reuse the same plan for RecordId materialization, metric work, and
// diagnostics without introducing cache invalidation semantics.
struct RegularSamplePlan {
	std::vector<std::size_t> positions;
	std::size_t record_count{};
	std::size_t requested_count{};
	std::size_t candidate_universe{};
	bool excludes_position{false};
	std::size_t excluded_position{};

	auto empty() const -> bool { return positions.empty(); }
	auto size() const -> std::size_t { return positions.size(); }
};

inline auto regular_sample_positions(std::size_t record_count, std::size_t sample_count) -> RegularSamplePlan
{
	RegularSamplePlan plan;
	plan.record_count = record_count;
	plan.requested_count = sample_count;
	plan.candidate_universe = record_count;
	if (record_count == 0 || sample_count == 0) {
		return plan;
	}

	sample_count = std::min(sample_count, record_count);
	plan.positions.reserve(sample_count);
	for (std::size_t sample_index = 0; sample_index < sample_count; ++sample_index) {
		plan.positions.push_back((sample_index * record_count) / sample_count);
	}
	return plan;
}

inline auto regular_sample_positions_excluding(std::size_t record_count, std::size_t excluded_position,
											   std::size_t sample_count) -> RegularSamplePlan
{
	RegularSamplePlan plan;
	plan.record_count = record_count;
	plan.requested_count = sample_count;
	plan.excludes_position = true;
	plan.excluded_position = excluded_position;
	plan.candidate_universe = record_count > 0 ? record_count - 1 : 0;
	if (record_count == 0 || sample_count == 0 || plan.candidate_universe == 0) {
		return plan;
	}
	if (excluded_position >= record_count) {
		throw std::out_of_range("excluded sample position is outside the record set");
	}

	sample_count = std::min(sample_count, plan.candidate_universe);
	plan.positions.reserve(sample_count);
	for (std::size_t sample_index = 0; sample_index < sample_count; ++sample_index) {
		const auto ordinal = (sample_index * plan.candidate_universe) / sample_count;
		plan.positions.push_back(ordinal < excluded_position ? ordinal : ordinal + 1);
	}
	return plan;
}

template <typename Provider>
auto record_ids_for_sample_plan(const Provider &provider, const RegularSamplePlan &plan) -> std::vector<RecordId>
{
	std::vector<RecordId> ids;
	ids.reserve(plan.positions.size());
	for (const auto position : plan.positions) {
		ids.push_back(provider.id(position));
	}
	return ids;
}

} // namespace mtrc::space

#endif
