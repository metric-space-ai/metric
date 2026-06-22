// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_INTENT_COMPRESS_HPP
#define _METRIC_INTENT_COMPRESS_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/space/storage/implicit.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/space/select/options.hpp>
#include <metric/space/select/representatives.hpp>

namespace mtrc::modify::reduce {
namespace detail {

template <typename Space, typename Provider>
auto compress_from_representatives(const Space &space, const Provider &provider,
								   const RepresentativeSet<typename Space::distance_type> &representatives)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;

	auto records = core::records_for_record_ids(space, representatives.representatives);

	auto representative_assignment = core::assign_records_to_representatives(
		provider, representatives.representatives, "representative set must not be empty",
		"representative id is outside compression provider");

	target_space_type compressed_space(std::move(records), space.metric());
	// compress is record-set CARDINALITY reduction: it keeps a coverage-based
	// subset of the original records under the source metric (so the metric law is
	// preserved) and maps every source record to its nearest representative. It is
	// not coordinate/dimension reduction.
	return core::make_compression_result(
		std::move(compressed_space), representatives.representatives,
		std::move(representative_assignment.assignments), std::move(representative_assignment.nearest_distances),
		space.size(), "representatives", representatives.strategy, representatives.representation, true, true, false,
		core::metric_traits<typename Space::metric_type>::law,
		"record-set cardinality reduction (coverage-based coarsening); kept records are an unmodified subset under "
		"the source metric; each source record maps to its nearest representative; not dimension reduction");
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, space::select::farthest_first strategy = {})
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (count == 0) {
		throw std::invalid_argument("compression count must be positive");
	}

	space::storage::LiveDistances<Space> provider(space);
	const auto representatives = find_representatives(space, count, strategy);
	return detail::compress_from_representatives(space, provider, representatives);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, space::select::farthest_first strategy,
			  space::storage::policy runtime_policy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (count == 0) {
		throw std::invalid_argument("compression count must be positive");
	}

	space::storage::require_exact_compress(runtime_policy);
	space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	if (runtime_policy.uses_materialization()) {
		space::storage::DistanceTable<Space> matrix(space);
		auto representatives = find_representatives(matrix, count, strategy);
		representatives.representation = space::storage::compression_representation(runtime_policy);
		return detail::compress_from_representatives(space, matrix, representatives);
	}

	auto result = compress(space, count, strategy);
	result.representation = space::storage::compression_representation(runtime_policy);
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto compress(const Space &space, std::size_t count, space::storage::policy runtime_policy)
	-> CompressionResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	return compress(space, count, space::select::farthest_first{}, runtime_policy);
}

} // namespace mtrc::modify::reduce

namespace mtrc {
using modify::reduce::compress;
} // namespace mtrc

#endif
