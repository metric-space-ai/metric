// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_NEAREST_HPP
#define _METRIC_OPERATORS_NEAREST_HPP

#include <algorithm>
#include <cstddef>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>

namespace mtrc::stats::search {

namespace batch_detail {

// Fill results[i] = f(i) for i in [0, n). Each batch query is independent (no
// shared mutable state), so the work is embarrassingly parallel. We fork
// std::threads only when the metric is declared thread-safe (concurrent const
// reads of the space then call only the thread-safe metric) AND the batch is
// large enough to amortize thread setup; otherwise it runs serially and is
// byte-identical to the single-query path. An exception in any worker is
// captured and rethrown after join, preserving the serial throw contract.
template <typename Result, typename Fn> auto run_parallel(std::size_t n, bool thread_safe, Fn f) -> std::vector<Result>
{
	std::vector<Result> results(n);
	const unsigned hardware = std::thread::hardware_concurrency();
	if (!thread_safe || n < 64 || hardware < 2) {
		for (std::size_t index = 0; index < n; ++index) {
			results[index] = f(index);
		}
		return results;
	}

	const unsigned workers = static_cast<unsigned>(std::min<std::size_t>(hardware, n));
	const std::size_t chunk = (n + workers - 1) / workers;
	std::vector<std::thread> pool;
	pool.reserve(workers);
	std::exception_ptr first_error;
	std::mutex error_mutex;

	for (unsigned worker = 0; worker < workers; ++worker) {
		const std::size_t begin = static_cast<std::size_t>(worker) * chunk;
		const std::size_t end = (begin + chunk < n) ? begin + chunk : n;
		if (begin >= end) {
			break;
		}
		pool.emplace_back([&results, &f, &first_error, &error_mutex, begin, end]() {
			try {
				for (std::size_t index = begin; index < end; ++index) {
					results[index] = f(index);
				}
			} catch (...) {
				std::lock_guard<std::mutex> guard(error_mutex);
				if (!first_error) {
					first_error = std::current_exception();
				}
			}
		});
	}
	for (auto &thread : pool) {
		thread.join();
	}
	if (first_error) {
		std::rethrow_exception(first_error);
	}
	return results;
}

} // namespace batch_detail

// Level-1 search investigates an existing finite metric space; it never defines a metric
// or modifies the space. knn() returns the k nearest records to a query (RecordId queries
// exclude the query itself); range() returns every record within a radius. Both assume a
// true metric (finite, non-negative distances). Distances are ordered ascending with
// RecordId-ascending tie-breaking, so results are deterministic for duplicate/tied records.
// A degenerate metric that emits a NaN distance does not invoke undefined behavior
// (NaN-distance neighbors are ordered last) but such an input is outside the metric contract.

namespace engine_detail {

template <typename Radius> auto validate_radius(Radius radius) -> void
{
	if (radius < Radius{}) {
		throw std::invalid_argument("radius must be non-negative");
	}
}

} // namespace engine_detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto knn(const Space &space, const typename Space::record_type &query, std::size_t k)
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	auto candidates = core::neighbor_candidates<distance_type>(
		space.size(), [&space](std::size_t index) { return space.id(index); },
		[&space, &query](RecordId id, std::size_t) { return space.metric()(query, space.record(id)); });

	return core::nearest_neighbor_set(std::move(candidates), k, space.size(), "metric_space");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto knn(const Space &space, RecordId query_id, std::size_t k) -> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	(void)space.record(query_id);
	auto candidates = core::neighbor_candidates_if<distance_type>(
		space.size(), [&space](std::size_t index) { return space.id(index); },
		[&space, query_id](RecordId id, std::size_t) { return space.distance(query_id, id); },
		[query_id](RecordId id, std::size_t) { return id != query_id; });

	return core::nearest_neighbor_set(std::move(candidates), k, space.size(), "metric_space");
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto knn(const Provider &provider, RecordId query_id, std::size_t k) -> NeighborSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (!provider.contains(query_id)) {
		throw std::out_of_range("query record id is outside the distance provider");
	}

	auto candidates = core::neighbor_candidates_if<distance_type>(
		provider.record_count(), [&provider](std::size_t index) { return provider.id(index); },
		[&provider, query_id](RecordId id, std::size_t) { return provider.distance(query_id, id); },
		[query_id](RecordId id, std::size_t) { return id != query_id; });

	return core::nearest_neighbor_set(std::move(candidates), k, provider.record_count(), "pairwise_distances");
}

template <typename Index, typename std::enable_if<NeighborSearchIndex_v<Index>, int>::type = 0>
auto knn(const Index &index, const typename Index::record_type &query, std::size_t k)
	-> NeighborSet<typename Index::distance_type>
{
	return core::make_neighbor_set(index.knn(query, k), index.record_count(), k, "knn", "neighbor_index");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, const typename Space::record_type &query, Radius radius)
	-> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	engine_detail::validate_radius(radius);
	auto candidates = core::neighbor_candidates_within<distance_type>(
		space.size(), [&space](std::size_t index) { return space.id(index); },
		[&space, &query](RecordId id, std::size_t) { return space.metric()(query, space.record(id)); }, radius);

	return core::range_neighbor_set(std::move(candidates), space.size(), "metric_space");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range(const Space &space, RecordId query_id, Radius radius) -> NeighborSet<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;

	engine_detail::validate_radius(radius);
	(void)space.record(query_id);
	auto candidates = core::neighbor_candidates_within_if<distance_type>(
		space.size(), [&space](std::size_t index) { return space.id(index); },
		[&space, query_id](RecordId id, std::size_t) { return space.distance(query_id, id); }, radius,
		[query_id](RecordId id, std::size_t) { return id != query_id; });

	return core::range_neighbor_set(std::move(candidates), space.size(), "metric_space");
}

template <typename Provider, typename Radius, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto range(const Provider &provider, RecordId query_id, Radius radius) -> NeighborSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	engine_detail::validate_radius(radius);
	if (!provider.contains(query_id)) {
		throw std::out_of_range("query record id is outside the distance provider");
	}
	auto candidates = core::neighbor_candidates_within_if<distance_type>(
		provider.record_count(), [&provider](std::size_t index) { return provider.id(index); },
		[&provider, query_id](RecordId id, std::size_t) { return provider.distance(query_id, id); }, radius,
		[query_id](RecordId id, std::size_t) { return id != query_id; });

	return core::range_neighbor_set(std::move(candidates), provider.record_count(), "pairwise_distances");
}

// Batch queries answer the same Level-1 investigation for many queries at once and return
// one NeighborSet per query, in input order. Each result keeps its own exactness and
// representation metadata. There is no shared state between queries, so the i-th result is
// identical to calling the corresponding single-query overload directly (same RecordId tie
// policy, same self-exclusion for RecordId queries).
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto knn_batch(const Space &space, const std::vector<typename Space::record_type> &queries, std::size_t k)
	-> std::vector<NeighborSet<typename Space::distance_type>>
{
	using result_type = NeighborSet<typename Space::distance_type>;
	constexpr bool thread_safe = core::metric_thread_safe_v<typename Space::metric_type>;
	return batch_detail::run_parallel<result_type>(queries.size(), thread_safe,
												   [&](std::size_t index) { return knn(space, queries[index], k); });
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto knn_batch(const Space &space, const std::vector<RecordId> &query_ids, std::size_t k)
	-> std::vector<NeighborSet<typename Space::distance_type>>
{
	using result_type = NeighborSet<typename Space::distance_type>;
	constexpr bool thread_safe = core::metric_thread_safe_v<typename Space::metric_type>;
	return batch_detail::run_parallel<result_type>(query_ids.size(), thread_safe,
												   [&](std::size_t index) { return knn(space, query_ids[index], k); });
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range_batch(const Space &space, const std::vector<typename Space::record_type> &queries, Radius radius)
	-> std::vector<NeighborSet<typename Space::distance_type>>
{
	engine_detail::validate_radius(radius);
	using result_type = NeighborSet<typename Space::distance_type>;
	constexpr bool thread_safe = core::metric_thread_safe_v<typename Space::metric_type>;
	return batch_detail::run_parallel<result_type>(queries.size(), thread_safe,
												   [&](std::size_t index) { return range(space, queries[index], radius); });
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto range_batch(const Space &space, const std::vector<RecordId> &query_ids, Radius radius)
	-> std::vector<NeighborSet<typename Space::distance_type>>
{
	engine_detail::validate_radius(radius);
	using result_type = NeighborSet<typename Space::distance_type>;
	constexpr bool thread_safe = core::metric_thread_safe_v<typename Space::metric_type>;
	return batch_detail::run_parallel<result_type>(query_ids.size(), thread_safe,
												   [&](std::size_t index) { return range(space, query_ids[index], radius); });
}

} // namespace mtrc::stats::search

#endif
