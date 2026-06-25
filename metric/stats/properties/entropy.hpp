// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_ENTROPY_HPP
#define _METRIC_OPERATORS_ENTROPY_HPP

#include <cmath>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <metric/core/concepts.hpp>
#include <metric/core/result.hpp>
#include <metric/correlation/entropy.hpp>

namespace mtrc::stats::properties {

namespace entropy_detail {

// Replicates the (k, p) clamping the kpN estimator (mtrc::Entropy::operator()) performs
// internally for small finite spaces, so the EntropyResult can report the EFFECTIVE
// neighbor count and approximation order instead of only the requested ones. Precondition:
// record_count >= 4 (the estimator's minimum; below that no estimation is attempted). Kept
// in lockstep with metric/correlation/entropy.cpp; any change there must change this.
struct effective_parameters {
	std::size_t neighbor_count{};
	std::size_t approximation_order{};
};

inline auto entropy_effective_parameters(std::size_t record_count, std::size_t k, std::size_t p)
	-> effective_parameters
{
	// Mirrors metric/correlation/entropy.cpp exactly (record_count >= 4 here, so p_ = n - 1
	// cannot underflow, and a sane request keeps p_ >= 1 before k_ = p_ - 1).
	std::size_t k_ = k;
	std::size_t p_ = p;
	if (p_ >= record_count) {
		p_ = record_count - 1;
	}
	if (k_ >= p_) {
		k_ = p_ - 1;
	}
	if (p_ < 3) {
		p_ = 3;
	}
	if (k_ < 2) {
		k_ = 2;
	}
	return {k_, p_};
}

// Entropy is a Level-1 PROPERTY of a coordinate finite metric space, not a source-metric
// shortcut. The current kpN estimator uses the space metric for local-neighborhood search,
// but it also fits local Gaussian volumes from coordinate-like records. In the METRIC
// pipeline this means: first map/embed the source finite metric space into a coordinate
// space with an appropriate Euclidean-style metric, then run entropy on that derived
// space. A true provider-only entropy estimator over arbitrary PairwiseDistances would
// need a different implementation instead of a wrapper around this one.
//
// The estimator (mtrc::Entropy) is a kpN local-Gaussian, Kozachenko-Leonenko-style
// DIFFERENTIAL entropy estimator (hal-01272527). Assumptions and edge cases callers
// must know:
//
//   * The input is the embedded/coordinate space; do not call entropy directly on source
//     records plus their domain metric.
//   * The derived space metric is the authoritative local-neighborhood distance; the
//     default record family uses Chebyshev.
//   * Records must expose size() and operator[](index); arbitrary opaque records and
//     precomputed distance providers are outside this estimator's contract.
//   * `k` is the nearest-neighbor count and `p` the local approximation order
//     (realizations used to fit each local Gaussian). For small spaces both are
//     silently clamped internally (p -> min(p, n-1) and >= 3, k -> < p and >= 2). The
//     result reports BOTH the requested neighbor_count/approximation_order and the
//     EFFECTIVE effective_neighbor_count/effective_approximation_order actually used.
//   * At least 4 records are required. For n < 4 (including an empty space) the
//     estimate is the NaN sentinel std::nan("estimation failed"). The result carries an
//     explicit `status` (entropy_status): `valid`, `too_few_records` (n < 4),
//     `degenerate` (no point yielded a valid local Gaussian -- e.g. a zero-diameter
//     space whose records are all identical), or `estimator_failure` (any other
//     non-finite estimate). Prefer result.succeeded()/result.status over a raw isnan
//     check; the `exact` flag does NOT encode failure.
//   * The result is a DIFFERENTIAL entropy in nats and CAN BE NEGATIVE. A negative value
//     is `valid`, never a failure. With `exponentiated = true`, values below 1 are
//     remapped onto a positive exponential scale (order preserving).
//   * The estimate is translation-invariant but is not guaranteed invariant to record
//     ordering when neighbor distances tie.
template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto coordinate_entropy(const Container &records, const Metric &metric, std::size_t k, std::size_t p,
						bool exponentiated, const char *representation) -> EntropyResult<double>
{
	static_assert(CoordinateRecordLike_v<Record>,
				  "mtrc::entropy requires an embedded coordinate space whose records expose size() and operator[]");
	mtrc::Entropy<void, Metric> estimator(metric, k, p, exponentiated);

	auto result = core::make_entropy_result(estimator(records), records.size(), k, p, exponentiated, representation);

	// Classify the outcome explicitly (see entropy_status). A finite value -- including a
	// negative differential entropy -- is valid; the estimator's std::nan("estimation
	// failed") sentinel is too_few_records below n == 4 and degenerate at or above it.
	const auto record_count = records.size();
	if (record_count < 4) {
		result.status = core::entropy_status::too_few_records;
		result.effective_neighbor_count = 0;
		result.effective_approximation_order = 0;
	} else {
		const auto effective = entropy_effective_parameters(record_count, k, p);
		result.effective_neighbor_count = effective.neighbor_count;
		result.effective_approximation_order = effective.approximation_order;
		if (std::isnan(result.value)) {
			result.status = core::entropy_status::degenerate;
		} else if (!std::isfinite(result.value)) {
			result.status = core::entropy_status::estimator_failure;
		} else {
			result.status = core::entropy_status::valid;
		}
	}

	return result;
}

} // namespace entropy_detail

template <typename Space, typename std::enable_if<CoordinateSpaceLike_v<Space>, int>::type = 0>
auto entropy(const Space &space, std::size_t k = 7, std::size_t p = 70, bool exponentiated = false)
	-> EntropyResult<double>
{
	auto result = entropy_detail::coordinate_entropy(space.records(), space.metric(), k, p, exponentiated,
													 "metric_space");
	result.representation = "metric_space";
	return result;
}

template <typename Space>
auto entropy(const core::MappingResult<Space> &mapping, std::size_t k = 7, std::size_t p = 70,
			 bool exponentiated = false) -> EntropyResult<double>
{
	core::require_mapping_result_contract(mapping, "entropy(mapping)");
	static_assert(CoordinateSpaceLike_v<Space>,
				  "mtrc::entropy requires a MappingResult whose target space is an embedded coordinate space");
	auto result = entropy(mapping.space, k, p, exponentiated);
	result.representation = mapping.representation.empty() ? "mapped_metric_space" : mapping.representation;
	return result;
}

} // namespace mtrc::stats::properties

namespace mtrc {
// Re-export so the entropy verb sits at the same `mtrc::` depth as the other
// space-level verbs (find_neighbors, find_groups, describe_structure, ...).
using stats::properties::entropy;
} // namespace mtrc

#endif
