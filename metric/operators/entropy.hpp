// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_OPERATORS_ENTROPY_HPP
#define _METRIC_OPERATORS_ENTROPY_HPP

#include <cstddef>
#include <type_traits>

#include "../core/concepts.hpp"
#include "../core/result.hpp"
#include "../correlation/entropy.hpp"

namespace metric::operators {

template <typename Container, typename Metric,
		  typename Record = typename std::decay<typename Container::value_type>::type,
		  typename std::enable_if<MetricCallable_v<Metric, Record>, int>::type = 0>
auto entropy(const Container &records, const Metric &metric, std::size_t k = 7, std::size_t p = 70,
			 bool exponentiated = false) -> EntropyResult<double>
{
	metric::Entropy<void, Metric> estimator(metric, k, p, exponentiated);

	EntropyResult<double> result;
	result.value = estimator(records);
	result.record_count = records.size();
	result.neighbor_count = k;
	result.approximation_order = p;
	result.exponentiated = exponentiated;
	result.exact = true;
	result.algorithm = "entropy";
	result.representation = "records";
	return result;
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto entropy(const Space &space, std::size_t k = 7, std::size_t p = 70, bool exponentiated = false)
	-> EntropyResult<double>
{
	auto result = entropy(space.records(), space.metric(), k, p, exponentiated);
	result.representation = "metric_space";
	return result;
}

} // namespace metric::operators

#endif
