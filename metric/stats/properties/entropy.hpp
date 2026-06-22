// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_ENTROPY_HPP
#define _METRIC_OPERATORS_ENTROPY_HPP

#include <cstddef>
#include <type_traits>

#include <metric/core/concepts.hpp>
#include <metric/core/result.hpp>
#include <metric/correlation/entropy.hpp>

namespace mtrc::stats::properties {

// Entropy is a Level-1 PROPERTY of a finite metric space, not a metric. It estimates
// the information content / local freedom of the space directly from metric values, so
// the chosen metric stays part of the diagnostic instead of being hidden behind a
// vector embedding.
//
// The estimator (mtrc::Entropy) is a kpN local-Gaussian, Kozachenko-Leonenko-style
// DIFFERENTIAL entropy estimator (hal-01272527). Assumptions and edge cases callers
// must know:
//
//   * `metric` is the authoritative distance; the default record family uses Chebyshev.
//   * `k` is the nearest-neighbor count and `p` the local approximation order
//     (realizations used to fit each local Gaussian). For small spaces both are
//     silently clamped internally (p -> min(p, n-1) and >= 3, k -> < p and >= 2); the
//     reported neighbor_count/approximation_order are the REQUESTED values, which may
//     therefore differ from the effective ones used.
//   * At least 4 records are required. For n < 4 (including an empty space) the
//     estimate is the NaN sentinel std::nan("estimation failed"); callers should test
//     std::isnan(result.value) to detect failure (the `exact` flag does not encode it).
//     The same NaN sentinel is returned when no point yields a valid local Gaussian --
//     e.g. a zero-diameter space whose records are all identical -- since there is then
//     no local geometry to estimate from.
//   * The result is a DIFFERENTIAL entropy in nats and CAN BE NEGATIVE. With
//     `exponentiated = true`, values below 1 are remapped onto a positive exponential
//     scale (order preserving).
//   * The estimate is translation-invariant but is not guaranteed invariant to record
//     ordering when neighbor distances tie.
template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto entropy(const Container &records, const Metric &metric, std::size_t k = 7, std::size_t p = 70,
			 bool exponentiated = false) -> EntropyResult<double>
{
	mtrc::Entropy<void, Metric> estimator(metric, k, p, exponentiated);

	return core::make_entropy_result(estimator(records), records.size(), k, p, exponentiated, "records");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto entropy(const Space &space, std::size_t k = 7, std::size_t p = 70, bool exponentiated = false)
	-> EntropyResult<double>
{
	auto result = entropy(space.records(), space.metric(), k, p, exponentiated);
	result.representation = "metric_space";
	return result;
}

} // namespace mtrc::stats::properties

#endif
