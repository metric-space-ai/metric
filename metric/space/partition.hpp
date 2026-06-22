// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_PARTITION_HPP
#define _METRIC_SPACE_PARTITION_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/record/id.hpp>

// Lineage-preserving container operations on a finite metric space.
//
// split() / select_subspace() carve a parent space into sub-spaces; merge() combines spaces into one. Each
// operation produces fresh mtrc::core::MetricSpace objects that reuse the parent metric (so metric validity is
// preserved) and reports, for every record in the result, the original RecordId it came from (so RecordId
// lineage is preserved across the partition/combination). These are pure structural operations: they do not
// analyse the space (no stats) and do not mutate the inputs (no modify).
namespace mtrc::space {

// A sub-space carved out of a parent space, together with the parent RecordId of every record it holds.
template <typename Record, typename Metric> struct Subspace {
	using space_type = core::MetricSpace<Record, Metric>;

	space_type space;
	// source_ids[local_position] == the RecordId the record had in the parent space.
	std::vector<RecordId> source_ids;
};

// Where a record in a merged space originated.
struct MergeOrigin {
	std::size_t source_index{}; // index of the input space, in argument order
	RecordId source_id{};       // RecordId the record had within that input space
};

template <typename Record, typename Metric> struct MergedSpace {
	using space_type = core::MetricSpace<Record, Metric>;

	space_type space;
	// lineage[new_position] -> origin of the record now at that position in `space`.
	std::vector<MergeOrigin> lineage;
};

// Build a sub-space from an explicit, ordered set of parent RecordIds. Throws std::out_of_range (via
// records_for_record_ids -> space.record) if any id is not present in the parent, so a sub-space can never
// silently reference a record that does not exist.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto select_subspace(const Space &parent, const std::vector<RecordId> &ids)
	-> Subspace<typename Space::record_type, typename Space::metric_type>
{
	using record_type = typename Space::record_type;
	using metric_type = typename Space::metric_type;

	auto records = mtrc::records_for_record_ids(parent, ids);
	return Subspace<record_type, metric_type>{
		core::MetricSpace<record_type, metric_type>(std::move(records), parent.metric()), ids};
}

// Split a parent space into one sub-space per group of RecordIds. Groups are independent: they may be any
// subset of the parent (the operation does not require them to be a disjoint cover), and every resulting
// sub-space keeps the parent metric and the lineage of its records.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto split(const Space &parent, const std::vector<std::vector<RecordId>> &groups)
	-> std::vector<Subspace<typename Space::record_type, typename Space::metric_type>>
{
	std::vector<Subspace<typename Space::record_type, typename Space::metric_type>> parts;
	parts.reserve(groups.size());
	for (const auto &group : groups) {
		parts.push_back(select_subspace(parent, group));
	}
	return parts;
}

// Two-way split by a predicate over the record. The first sub-space holds the records for which
// predicate(record) is true (in parent order), the second holds the rest; lineage is preserved for both.
template <typename Space, typename Predicate, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto split_if(const Space &parent, Predicate predicate)
	-> std::pair<Subspace<typename Space::record_type, typename Space::metric_type>,
				 Subspace<typename Space::record_type, typename Space::metric_type>>
{
	std::vector<RecordId> matched;
	std::vector<RecordId> rest;
	for (std::size_t position = 0; position < parent.size(); ++position) {
		const auto id = parent.id(position);
		if (predicate(parent.record(id))) {
			matched.push_back(id);
		} else {
			rest.push_back(id);
		}
	}
	return {select_subspace(parent, matched), select_subspace(parent, rest)};
}

// Merge several spaces (sharing record and metric types) into one. The merged space reuses the metric of the
// first input; records are concatenated in input order and lineage records, for each merged record, which
// input space and original RecordId it came from.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto merge(const std::vector<const Space *> &spaces)
	-> MergedSpace<typename Space::record_type, typename Space::metric_type>
{
	using record_type = typename Space::record_type;
	using metric_type = typename Space::metric_type;

	std::vector<record_type> records;
	std::vector<MergeOrigin> lineage;
	bool have_metric = false;
	metric_type metric{};

	for (std::size_t source_index = 0; source_index < spaces.size(); ++source_index) {
		const Space *source = spaces[source_index];
		if (source == nullptr) {
			continue;
		}
		if (!have_metric) {
			metric = source->metric();
			have_metric = true;
		}
		for (std::size_t position = 0; position < source->size(); ++position) {
			const auto id = source->id(position);
			records.push_back(source->record(id));
			lineage.push_back(MergeOrigin{source_index, id});
		}
	}

	return MergedSpace<record_type, metric_type>{
		core::MetricSpace<record_type, metric_type>(std::move(records), std::move(metric)), std::move(lineage)};
}

// Convenience two-argument merge.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto merge(const Space &lhs, const Space &rhs)
	-> MergedSpace<typename Space::record_type, typename Space::metric_type>
{
	return merge(std::vector<const Space *>{&lhs, &rhs});
}

} // namespace mtrc::space

namespace mtrc {
using space::merge;
using space::MergedSpace;
using space::MergeOrigin;
using space::select_subspace;
using space::split;
using space::split_if;
using space::Subspace;
} // namespace mtrc

#endif
