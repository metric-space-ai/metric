// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_DISTANCES_HPP
#define _METRIC_SPACE_DISTANCES_HPP

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/neighbor.hpp>
#include <metric/record/id.hpp>
#include <metric/space/storage/distance_table.hpp>
#include <metric/space/storage/diagnostics.hpp>
#include <metric/space/storage/implicit.hpp>

// User-facing access to the pairwise metric values of a finite metric space.
//
// mtrc::space::distances is the one place a caller goes for "the distance between these two records"
// and "every distance from this record", without having to know whether the values are produced live
// from the metric, served from a materialized table, or read from any other storage form. Everything
// here works against either:
//
//   * a mtrc::core::MetricSpace directly (values are computed live, always fresh), or
//   * a pairwise-distance provider from mtrc::space::storage (LiveDistances, DistanceTable, ...), which
//     may be materialized and may go stale after the source space mutates.
//
// The provider helpers (live / materialize / lazy_table) construct the existing storage components;
// this namespace adds no new storage or algorithm, it only gives them an ergonomic, representation-
// neutral surface. Stale handling is explicit: require_fresh / checked_value refuse to serve values
// from a provider that was built for an older space version, so a mutation can never silently return
// outdated distances (see also mtrc::space::cache for rebuild helpers). The unchecked accessors
// (value / row / pairs / for_each_pair) do not guard staleness: on a stale provider they return
// whatever it yields, and if the source space has since shrunk a lazy provider may re-evaluate an
// erased record and throw std::out_of_range. Route through checked_value / require_fresh when a single
// well-defined StaleRepresentationError is preferred over either stale numbers or that throw.
namespace mtrc::space::distances {

// One pairwise value, tagged with the two records it relates.
template <typename Distance> struct pair_value {
	RecordId lhs;
	RecordId rhs;
	Distance distance{};
};

inline constexpr std::size_t default_max_collected_pair_values = 1'000'000;

struct pair_collection_options {
	// Maximum number of unordered pair values that pairs(...) may materialize in memory.
	// Set to 0 only when the caller intentionally opts into an unbounded collection.
	std::size_t max_pair_values{default_max_collected_pair_values};
};

// Compact status of a pairwise-value source: is it materialized, is it exact, and has it gone stale?
struct provider_status {
	bool materialized{};
	bool exact{};
	bool stale{};
	std::size_t record_count{};
	std::size_t built_for_version{};
	std::size_t space_version{};
};

// ---------------------------------------------------------------------------------------------
// Provider construction (reuses mtrc::space::storage; constructs nothing new).
// ---------------------------------------------------------------------------------------------

// Lazy, always-fresh provider: every value is recomputed from the metric on demand. Never stale.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto live(const Space &space) -> storage::LiveDistances<Space>
{
	return storage::LiveDistances<Space>(space);
}

// Materialized provider: an eager all-pairs distance table built once for the current space version.
// Reuses cached values on access; goes stale when the space mutates.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto materialize(const Space &space) -> storage::DistanceTable<Space>
{
	return storage::DistanceTable<Space>(space, storage::distance_table_mode::eager);
}

// Lazy table provider: a distance table that caches values the first time each pair is requested.
// Trades a smaller up-front cost for per-pair caching; goes stale when the space mutates.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto lazy_table(const Space &space) -> storage::DistanceTable<Space>
{
	return storage::DistanceTable<Space>(space, storage::distance_table_mode::lazy);
}

// ---------------------------------------------------------------------------------------------
// Stale handling.
// ---------------------------------------------------------------------------------------------

// True iff the provider was built for an older space version than its source space now has.
// A live space (no is_stale member) is treated as never stale.
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto is_stale(const Provider &provider) -> bool
{
	return provider.is_stale();
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto is_stale(const Space &) -> bool
{
	return false;
}

// Throw StaleRepresentationError if the provider is stale. Use before serving cached values so a
// mutation never silently yields outdated distances.
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto require_fresh(const Provider &provider,
				   const char *message = "pairwise-distance provider is stale: it was built for an older "
										 "metric-space version; rebuild it before reading distances") -> void
{
	if (provider.is_stale()) {
		throw StaleRepresentationError(message);
	}
}

// ---------------------------------------------------------------------------------------------
// Pair-value access.
// ---------------------------------------------------------------------------------------------

// Distance between two records, from a provider. No staleness check (callers that want one use
// checked_value). On a stale provider this returns whatever the provider yields: a cached value from
// before the last mutation if it has one, otherwise the provider may re-evaluate against the source
// space and, if the requested record was erased since the provider was built, that re-evaluation
// throws std::out_of_range. Use checked_value / require_fresh to convert staleness into a single
// well-defined StaleRepresentationError up front.
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto value(const Provider &provider, RecordId lhs, RecordId rhs) -> typename Provider::distance_type
{
	return provider.distance(lhs, rhs);
}

// Distance between two records, computed live from the space. Always fresh.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto value(const Space &space, RecordId lhs, RecordId rhs) -> typename Space::distance_type
{
	return space.distance(lhs, rhs);
}

// Distance between two records from a provider, refusing to serve a stale provider. Throws
// StaleRepresentationError if the provider is out of date.
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto checked_value(const Provider &provider, RecordId lhs, RecordId rhs) -> typename Provider::distance_type
{
	require_fresh(provider);
	return provider.distance(lhs, rhs);
}

// ---------------------------------------------------------------------------------------------
// Row access: every distance from one record.
// ---------------------------------------------------------------------------------------------

// Distances from `source` to every record in the provider, in provider position order (the source's
// own zero-distance self-pair is included). Reuses the provider's id()/distance() surface, so it works
// identically over live and materialized sources.
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto row(const Provider &provider, RecordId source) -> std::vector<Neighbor<typename Provider::distance_type>>
{
	using distance_type = typename Provider::distance_type;
	if (!provider.contains(source)) {
		throw std::out_of_range("row source record id is outside the distance provider");
	}
	std::vector<Neighbor<distance_type>> values;
	values.reserve(provider.record_count());
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto id = provider.id(index);
		values.push_back(Neighbor<distance_type>{id, provider.distance(source, id)});
	}
	return values;
}

// Row over a live space (computed from the metric).
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto row(const Space &space, RecordId source) -> std::vector<Neighbor<typename Space::distance_type>>
{
	return row(storage::LiveDistances<Space>(space), source);
}

// ---------------------------------------------------------------------------------------------
// Symmetric pair iteration over the unordered record pairs {i, j} with i < j (each pair once).
// ---------------------------------------------------------------------------------------------

// Invoke visitor(lhs_id, rhs_id, distance) for every unordered pair of distinct records (i < j in
// provider position order). For a true (symmetric) metric this visits each pair's value exactly once.
template <typename Provider, typename Visitor,
		  typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto for_each_pair(const Provider &provider, Visitor visitor) -> void
{
	const auto count = provider.record_count();
	for (std::size_t lhs = 0; lhs < count; ++lhs) {
		const auto lhs_id = provider.id(lhs);
		for (std::size_t rhs = lhs + 1; rhs < count; ++rhs) {
			const auto rhs_id = provider.id(rhs);
			visitor(lhs_id, rhs_id, provider.distance(lhs_id, rhs_id));
		}
	}
}

template <typename Space, typename Visitor,
		  typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto for_each_pair(const Space &space, Visitor visitor) -> void
{
	for_each_pair(storage::LiveDistances<Space>(space), std::move(visitor));
}

inline auto unordered_pair_count(std::size_t count) -> std::size_t
{
	if (count < 2) {
		return 0;
	}
	const auto lhs = count % 2 == 0 ? count / 2 : count;
	const auto rhs = count % 2 == 0 ? count - 1 : (count - 1) / 2;
	if (lhs > std::numeric_limits<std::size_t>::max() / rhs) {
		throw RepresentationError("space::distances::pairs cannot represent C(record_count, 2) in std::size_t");
	}
	return lhs * rhs;
}

inline auto require_pair_collection_budget(std::size_t record_count, pair_collection_options options) -> std::size_t
{
	const auto pair_count = unordered_pair_count(record_count);
	if (options.max_pair_values == 0 || pair_count <= options.max_pair_values) {
		return pair_count;
	}
	throw RepresentationError(
		std::string("space::distances::pairs refused to collect all unordered pairs before metric calls: records=") +
		std::to_string(record_count) + ", pair_count=" + std::to_string(pair_count) +
		", max_pair_values=" + std::to_string(options.max_pair_values) +
		". Use space::distances::for_each_pair for streaming iteration or pass pair_collection_options{0} "
		"to explicitly opt into an unbounded in-memory collection.");
}

// Collect every unordered pair {i, j}, i < j, with its distance.
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto pairs(const Provider &provider, pair_collection_options options = {})
	-> std::vector<pair_value<typename Provider::distance_type>>
{
	using distance_type = typename Provider::distance_type;
	std::vector<pair_value<distance_type>> collected;
	const auto count = provider.record_count();
	const auto pair_count = require_pair_collection_budget(count, options);
	collected.reserve(pair_count);
	for_each_pair(provider, [&collected](RecordId lhs, RecordId rhs, distance_type distance) {
		collected.push_back(pair_value<distance_type>{lhs, rhs, distance});
	});
	return collected;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto pairs(const Space &space, pair_collection_options options = {}) -> std::vector<pair_value<typename Space::distance_type>>
{
	return pairs(storage::LiveDistances<Space>(space), options);
}

// ---------------------------------------------------------------------------------------------
// Status / diagnostics.
// ---------------------------------------------------------------------------------------------

// Compact materialized/exact/stale status, derived from the provider's own diagnostics().
template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto status(const Provider &provider) -> provider_status
{
	const auto diagnostics = provider.diagnostics();
	provider_status result;
	result.materialized = diagnostics.materialized == storage::materialization::materialized;
	result.exact = diagnostics.exact == storage::exactness::exact;
	result.stale = diagnostics.stale;
	result.record_count = diagnostics.records;
	result.built_for_version = diagnostics.built_for_version;
	result.space_version = diagnostics.space_version;
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto status(const Space &space) -> provider_status
{
	return status(storage::LiveDistances<Space>(space));
}

} // namespace mtrc::space::distances

#endif
