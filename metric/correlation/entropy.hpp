/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_RANDOM_ENTROPY_HPP
#define _METRIC_DISTANCE_K_RANDOM_ENTROPY_HPP

#include <cstddef>

#include <metric/metric/catalog/vector/Standards.hpp>
#include "metric/utils/type_traits.hpp"
#include <metric/numeric.hpp>

namespace mtrc {

struct entropy_resource_options {
	std::size_t max_exact_records = 4096;
	std::size_t max_distance_evaluations = 100'000'000;
};

// non-kpN version, DEPRECATED
template <typename RecType, typename Metric = mtrc::Euclidean<typename RecType::value_type>>
class EntropySimple { // averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO
					  // refactor to avoid code dubbing
  public:
	EntropySimple(Metric metric = Metric(), size_t k = 3, bool exp = false,
				  entropy_resource_options resource_options = {})
		: k(k), metric(metric), logbase(2), exp(exp), resource_options(resource_options)
	{
	} // TODO remove (?)

	template <typename Container> double operator()(const Container &data) const;
	// double operator()(const Container& data, bool avoid_repeated = false) const;

	template <template <typename, typename> class OuterContainer, template <typename, bool> class InnerContainer,
			  class OuterAllocator, typename ValueType, bool F>
	double operator()( // TODO implement
		const OuterContainer<InnerContainer<ValueType, F>, OuterAllocator>
			&data // inner cpntainer is specialized with bool F

	) const;

	template <typename Container>
	double estimate(const Container &a, const size_t sampleSize = 250, const double threshold = 0.05,
					size_t maxIterations = 1000) const;

  private:
	size_t k;
	size_t p;
	Metric metric;
	double logbase;
	bool exp;
	entropy_resource_options resource_options;
};

// kpN local-Gaussian (Kozachenko-Leonenko style) DIFFERENTIAL entropy estimator of a
// finite metric space. https://hal.inria.fr/hal-01272527/document
// Notes: returns nats (no logbase rescaling, unlike the deprecated EntropySimple which
// uses base 2); the result can be negative; requires at least 4 records (returns
// std::nan("estimation failed") otherwise); k (neighbors) and p (local realizations)
// are clamped internally for small spaces. Also returns the same NaN sentinel when no
// point yields a valid local Gaussian (e.g. a zero-diameter space of identical records),
// so callers should test std::isnan on the result. See mtrc::stats::properties::entropy.
template <typename RecType, typename Metric = mtrc::Chebyshev<typename RecType::value_type>> class Entropy {
  public:
	Entropy(Metric metric = Metric(), size_t k = 7, size_t p = 70, bool exp = false,
			entropy_resource_options resource_options = {})
		: k(k), p(p), metric(metric), exp(exp), resource_options(resource_options)
	{
	}

	template <typename Container> double operator()(const Container &data) const;

	template <typename Container>
	double estimate(const Container &a, const size_t sampleSize = 250, const double threshold = 0.05,
					size_t maxIterations = 1000) const;

  private:
	size_t k;
	size_t p;
	Metric metric;
	bool exp;
	entropy_resource_options resource_options;
};

// VMixing

template <typename RecType, typename Metric = mtrc::Euclidean<typename RecType::value_type>>
class VMixing_simple { // non-kpN version, DEPRECATED

  public:
	VMixing_simple(Metric metric = Metric(), int k = 3, entropy_resource_options resource_options = {})
		: k(k), metric(metric), resource_options(resource_options)
	{
	}

	template <typename C>
	typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, type_traits::underlying_type_t<C>>
	operator()(const C &Xc, const C &Yc) const;

	template <typename C>
	double estimate(const C &a, const C &b, const size_t sampleSize = 250, const double threshold = 0.05,
					size_t maxIterations = 1000) const;

  private:
	int k;
	Metric metric;
	entropy_resource_options resource_options;
};

template <typename RecType, typename Metric = mtrc::Euclidean<typename RecType::value_type>> class VMixing {

  public:
	VMixing(Metric metric = Metric(), int k = 3, int p = 25, entropy_resource_options resource_options = {})
		: k(k), p(p), metric(metric), resource_options(resource_options)
	{
	}

	template <typename C>
	typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, type_traits::underlying_type_t<C>>
	operator()(const C &Xc, const C &Yc) const;

	template <typename C>
	double estimate(const C &a, const C &b, const size_t sampleSize = 250, const double threshold = 0.05,
					size_t maxIterations = 1000) const;

  private:
	int k;
	int p;
	Metric metric;
	entropy_resource_options resource_options;
};

/* // VOI code, works and may be enabled


template <typename C, typename Metric = mtrc::Chebyshev<type_traits::underlying_type_t<C>>>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, type_traits::underlying_type_t<C>>
VOI_simple(const C& Xc, const C& Yc, int k = 3);


template <typename C, typename Metric = mtrc::Chebyshev<type_traits::underlying_type_t<C>>>
typename std::enable_if_t<!type_traits::is_container_of_integrals_v<C>, type_traits::underlying_type_t<C>>
VOI(const C& Xc, const C& Yc, int k = 3, int p = 25);

// */

} // namespace mtrc

#include "entropy.cpp"
#endif
