// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// mtrc::modify::represent constructs a representative finite metric space: a
// LOSSLESS subset of the source records selected to cover the space.
//
// It is deliberately distinct from mtrc::modify::reduce::compress: represent
// only SELECTS records and keeps them unmodified under the source metric, with a
// one-to-one source lineage. It does not assign or collapse the non-selected
// records onto the representatives, so it produces no assignment map and no
// compression ratio. The derived space's metric law is exactly the source
// metric's law, because a metric restricted to a subset is still a metric.

#ifndef _METRIC_MODIFY_REPRESENT_HPP
#define _METRIC_MODIFY_REPRESENT_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>
#include <metric/space/select/options.hpp>
#include <metric/space/select/representatives.hpp>
#include <metric/space/storage/execution.hpp>

namespace mtrc::modify::represent {
namespace detail {

template <typename Space>
auto represent_from_set(const Space &space, const RepresentativeSet<typename Space::distance_type> &representatives)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;

	const auto &ids = representatives.representatives;
	auto records = core::records_for_record_ids(space, ids);
	auto lineage = ::mtrc::one_to_one_lineage(ids);

	target_space_type derived_space(std::move(records), space.metric());
	return core::make_mapping_result(
		std::move(derived_space), std::move(lineage.source_records), std::move(lineage.representative_records),
		space.size(), false, "represent", representatives.strategy, representatives.representation,
		core::metric_traits<typename Space::metric_type>::law, false,
		"representative subspace; selected records only; kept records are an unmodified subset under the source "
		"metric; non-selected records are not assigned or collapsed");
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto represent(const Space &space, std::size_t count, space::select::farthest_first strategy = {})
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	// Reject the degenerate empty subspace for parity with modify::reduce::compress
	// (find_representatives returns an empty set for count == 0 rather than throwing).
	if (count == 0) {
		throw std::invalid_argument("represent count must be positive");
	}
	const auto representatives = find_representatives(space, count, strategy);
	return detail::represent_from_set(space, representatives);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto represent(const Space &space, std::size_t count, space::select::farthest_first strategy,
			   space::storage::policy runtime_policy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (count == 0) {
		throw std::invalid_argument("represent count must be positive");
	}
	const auto representatives = find_representatives(space, count, strategy, runtime_policy);
	return detail::represent_from_set(space, representatives);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto represent(const Space &space, std::size_t count, space::storage::policy runtime_policy)
	-> decltype(represent(space, count, space::select::farthest_first{}, runtime_policy))
{
	return represent(space, count, space::select::farthest_first{}, runtime_policy);
}

} // namespace mtrc::modify::represent

namespace mtrc {
using modify::represent::represent;
} // namespace mtrc

#endif
