/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef _METRIC_METRIC_CATALOG_DISTRIBUTION_PROBABILITY_HPP
#define _METRIC_METRIC_CATALOG_DISTRIBUTION_PROBABILITY_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>

#include <metric/core/metric_traits.hpp>
#include <metric/metric/detail/vector_input.hpp>

namespace mtrc::detail {

template <typename V, typename Container>
auto validate_probability_vector(const Container &values, const char *name, V sum_tolerance) -> V
{
	if (values.empty()) {
		throw std::invalid_argument(std::string(name) + " requires non-empty probability vectors");
	}
	V sum = 0;
	for (std::size_t i = 0; i < values.size(); ++i) {
		const auto value = static_cast<V>(values[i]);
		if (!std::isfinite(value)) {
			throw std::invalid_argument(std::string(name) + " requires finite probability entries");
		}
		if (value < V(0)) {
			throw std::invalid_argument(std::string(name) + " requires nonnegative probability entries");
		}
		sum += value;
	}
	if (std::abs(sum - V(1)) > sum_tolerance) {
		throw std::invalid_argument(std::string(name) + " requires probabilities that sum to 1");
	}
	return sum;
}

template <typename V> void validate_probability_tolerance(V tolerance, const char *name)
{
	if (!std::isfinite(tolerance) || tolerance < V(0)) {
		throw std::invalid_argument(std::string(name) + " requires a finite nonnegative sum tolerance");
	}
}

} // namespace mtrc::detail

namespace mtrc {

/**
 * Total Variation metric on aligned probability vectors.
 *
 * Formula:
 *   d(p, q) = 0.5 * sum_i |p_i - q_i|
 *
 * Both inputs must be finite nonnegative vectors on the same support and must
 * sum to 1 within the configured tolerance.
 */
template <typename V = double> class TotalVariation {
  public:
	using value_type = V;
	using distance_type = value_type;

	explicit TotalVariation(value_type sum_tolerance = value_type(1e-9)) : sum_tolerance_(sum_tolerance)
	{
		detail::validate_probability_tolerance(sum_tolerance_, "TotalVariation");
	}

	template <typename Container> auto operator()(const Container &p, const Container &q) const -> distance_type
	{
		detail::require_aligned_vector_inputs(p, q, "TotalVariation");
		detail::validate_probability_tolerance(sum_tolerance_, "TotalVariation");
		detail::validate_probability_vector<value_type>(p, "TotalVariation", sum_tolerance_);
		detail::validate_probability_vector<value_type>(q, "TotalVariation", sum_tolerance_);
		value_type sum = 0;
		for (std::size_t i = 0; i < p.size(); ++i) {
			sum += std::abs(static_cast<value_type>(p[i]) - static_cast<value_type>(q[i]));
		}
		return value_type(0.5) * sum;
	}

	auto sum_tolerance() const -> value_type { return sum_tolerance_; }

  private:
	value_type sum_tolerance_;
};

/**
 * Hellinger metric on aligned probability vectors.
 *
 * Formula:
 *   d(p, q) = sqrt(0.5 * sum_i (sqrt(p_i) - sqrt(q_i))^2)
 *
 * This is the root form. Squared Hellinger is not promoted as a metric.
 */
template <typename V = double> class Hellinger {
  public:
	using value_type = V;
	using distance_type = value_type;

	explicit Hellinger(value_type sum_tolerance = value_type(1e-9)) : sum_tolerance_(sum_tolerance)
	{
		detail::validate_probability_tolerance(sum_tolerance_, "Hellinger");
	}

	template <typename Container> auto operator()(const Container &p, const Container &q) const -> distance_type
	{
		detail::require_aligned_vector_inputs(p, q, "Hellinger");
		detail::validate_probability_tolerance(sum_tolerance_, "Hellinger");
		detail::validate_probability_vector<value_type>(p, "Hellinger", sum_tolerance_);
		detail::validate_probability_vector<value_type>(q, "Hellinger", sum_tolerance_);
		value_type sum = 0;
		for (std::size_t i = 0; i < p.size(); ++i) {
			const auto diff = std::sqrt(static_cast<value_type>(p[i])) - std::sqrt(static_cast<value_type>(q[i]));
			sum += diff * diff;
		}
		return std::sqrt(value_type(0.5) * sum);
	}

	auto sum_tolerance() const -> value_type { return sum_tolerance_; }

  private:
	value_type sum_tolerance_;
};

/**
 * Square-root Jensen-Shannon metric on aligned probability vectors.
 *
 * Formula:
 *   d(p, q) = sqrt(0.5 KL(p || m) + 0.5 KL(q || m)), m = (p + q) / 2
 *
 * Natural logarithms are used; this only scales the metric relative to base-2.
 * The raw Jensen-Shannon divergence is intentionally not exposed as a metric.
 */
template <typename V = double> class SqrtJensenShannon {
  public:
	using value_type = V;
	using distance_type = value_type;

	explicit SqrtJensenShannon(value_type sum_tolerance = value_type(1e-9)) : sum_tolerance_(sum_tolerance)
	{
		detail::validate_probability_tolerance(sum_tolerance_, "SqrtJensenShannon");
	}

	template <typename Container> auto operator()(const Container &p, const Container &q) const -> distance_type
	{
		detail::require_aligned_vector_inputs(p, q, "SqrtJensenShannon");
		detail::validate_probability_tolerance(sum_tolerance_, "SqrtJensenShannon");
		detail::validate_probability_vector<value_type>(p, "SqrtJensenShannon", sum_tolerance_);
		detail::validate_probability_vector<value_type>(q, "SqrtJensenShannon", sum_tolerance_);

		value_type divergence = 0;
		for (std::size_t i = 0; i < p.size(); ++i) {
			const auto left = static_cast<value_type>(p[i]);
			const auto right = static_cast<value_type>(q[i]);
			const auto midpoint = value_type(0.5) * (left + right);
			if (left > value_type(0)) {
				divergence += value_type(0.5) * left * std::log(left / midpoint);
			}
			if (right > value_type(0)) {
				divergence += value_type(0.5) * right * std::log(right / midpoint);
			}
		}
		return std::sqrt(std::max(value_type(0), divergence));
	}

	auto sum_tolerance() const -> value_type { return sum_tolerance_; }

  private:
	value_type sum_tolerance_;
};

} // namespace mtrc

namespace mtrc::core {

template <typename V> struct metric_traits<::mtrc::TotalVariation<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::TotalVariation<V> &metric) -> std::string
	{
		return std::string("mtrc::TotalVariation:sum_tolerance=") + std::to_string(metric.sum_tolerance());
	}
};

template <typename V> struct metric_traits<::mtrc::Hellinger<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::Hellinger<V> &metric) -> std::string
	{
		return std::string("mtrc::Hellinger:sum_tolerance=") + std::to_string(metric.sum_tolerance());
	}
};

template <typename V> struct metric_traits<::mtrc::SqrtJensenShannon<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::SqrtJensenShannon<V> &metric) -> std::string
	{
		return std::string("mtrc::SqrtJensenShannon:sum_tolerance=") + std::to_string(metric.sum_tolerance());
	}
};

} // namespace mtrc::core

#endif
