/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef _METRIC_METRIC_CATALOG_VECTOR_GEOMETRY_HPP
#define _METRIC_METRIC_CATALOG_VECTOR_GEOMETRY_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/metric_traits.hpp>
#include <metric/metric/detail/vector_input.hpp>

namespace mtrc {

/**
 * Weighted Minkowski metric on aligned finite real vectors.
 *
 * Formula:
 *   d(x, y) = (sum_i w_i |x_i - y_i|^p)^(1/p)
 *
 * The metric law requires every weight to be finite and strictly positive and
 * the exponent to be finite with p >= 1. Zero weights would collapse identity
 * of indiscernibles, so they are rejected instead of silently demoted.
 */
template <typename V = double> class WeightedMinkowski {
  public:
	using value_type = V;
	using distance_type = value_type;

	static_assert(std::is_floating_point<V>::value, "WeightedMinkowski requires a floating-point value type");

	explicit WeightedMinkowski(std::vector<value_type> weights, value_type p = value_type(2))
		: weights_(std::move(weights)), p_(p)
	{
		validate_parameters();
	}

	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type
	{
		detail::require_aligned_vector_inputs(a, b, "WeightedMinkowski");
		validate_parameters();
		if (a.size() != weights_.size()) {
			throw std::invalid_argument("WeightedMinkowski input dimension does not match weights");
		}

		distance_type sum = 0;
		for (std::size_t i = 0; i < weights_.size(); ++i) {
			const auto left = static_cast<distance_type>(a[i]);
			const auto right = static_cast<distance_type>(b[i]);
			if (!std::isfinite(left) || !std::isfinite(right)) {
				throw std::invalid_argument("WeightedMinkowski requires finite vector entries");
			}
			sum += weights_[i] * std::pow(std::abs(left - right), p_);
		}
		return std::pow(sum, value_type(1) / p_);
	}

	auto weights() const -> const std::vector<value_type> & { return weights_; }
	auto exponent() const -> value_type { return p_; }

  private:
	std::vector<value_type> weights_;
	value_type p_;

	void validate_parameters() const
	{
		if (weights_.empty()) {
			throw std::invalid_argument("WeightedMinkowski requires at least one positive weight");
		}
		if (!std::isfinite(p_) || p_ < value_type(1)) {
			throw std::invalid_argument("WeightedMinkowski requires a finite exponent p >= 1");
		}
		for (const auto weight : weights_) {
			if (!std::isfinite(weight) || weight <= value_type(0)) {
				throw std::invalid_argument("WeightedMinkowski requires finite positive weights");
			}
		}
	}
};

/**
 * Mahalanobis metric with a symmetric positive definite precision matrix.
 *
 * Formula:
 *   d(x, y) = sqrt((x - y)^T A (x - y))
 *
 * The constructor validates that A is finite, square, symmetric, and strictly
 * positive definite by a Cholesky factorization. Positive semidefinite matrices
 * are not admitted here because they collapse distinct vectors to zero.
 */
template <typename V = double> class Mahalanobis {
  public:
	using value_type = V;
	using distance_type = value_type;

	static_assert(std::is_floating_point<V>::value, "Mahalanobis requires a floating-point value type");

	explicit Mahalanobis(std::vector<std::vector<value_type>> precision_matrix,
						 value_type tolerance = value_type(-1))
		: precision_(std::move(precision_matrix)), tolerance_(tolerance)
	{
		validate_matrix();
	}

	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type
	{
		detail::require_aligned_vector_inputs(a, b, "Mahalanobis");
		if (a.size() != precision_.size()) {
			throw std::invalid_argument("Mahalanobis input dimension does not match precision matrix");
		}

		std::vector<distance_type> diff(precision_.size());
		for (std::size_t i = 0; i < precision_.size(); ++i) {
			const auto left = static_cast<distance_type>(a[i]);
			const auto right = static_cast<distance_type>(b[i]);
			if (!std::isfinite(left) || !std::isfinite(right)) {
				throw std::invalid_argument("Mahalanobis requires finite vector entries");
			}
			diff[i] = left - right;
		}

		distance_type squared = 0;
		for (std::size_t i = 0; i < precision_.size(); ++i) {
			for (std::size_t j = 0; j < precision_.size(); ++j) {
				squared += diff[i] * precision_[i][j] * diff[j];
			}
		}
		const auto tol = matrix_tolerance();
		if (squared < -tol) {
			throw std::runtime_error("Mahalanobis SPD matrix produced a negative quadratic form");
		}
		return std::sqrt(std::max(distance_type(0), squared));
	}

	auto precision_matrix() const -> const std::vector<std::vector<value_type>> & { return precision_; }
	auto tolerance() const -> value_type { return matrix_tolerance(); }

  private:
	std::vector<std::vector<value_type>> precision_;
	value_type tolerance_;

	auto matrix_tolerance() const -> value_type
	{
		if (tolerance_ >= value_type(0)) {
			return tolerance_;
		}
		value_type scale = value_type(1);
		for (const auto &row : precision_) {
			for (const auto value : row) {
				scale = std::max(scale, std::abs(value));
			}
		}
		return value_type(100) * std::numeric_limits<value_type>::epsilon() * scale *
			   static_cast<value_type>(std::max<std::size_t>(1, precision_.size()));
	}

	void validate_matrix() const
	{
		if (precision_.empty()) {
			throw std::invalid_argument("Mahalanobis requires a non-empty precision matrix");
		}
		if (!std::isfinite(tolerance_) && !(tolerance_ < value_type(0))) {
			throw std::invalid_argument("Mahalanobis requires a finite matrix tolerance");
		}
		const auto n = precision_.size();
		for (const auto &row : precision_) {
			if (row.size() != n) {
				throw std::invalid_argument("Mahalanobis requires a square precision matrix");
			}
			for (const auto value : row) {
				if (!std::isfinite(value)) {
					throw std::invalid_argument("Mahalanobis requires finite precision matrix entries");
				}
			}
		}

		const auto tol = matrix_tolerance();
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = i + 1; j < n; ++j) {
				if (std::abs(precision_[i][j] - precision_[j][i]) > tol) {
					throw std::invalid_argument("Mahalanobis requires a symmetric precision matrix");
				}
			}
		}

		std::vector<std::vector<value_type>> lower(n, std::vector<value_type>(n, value_type(0)));
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = 0; j <= i; ++j) {
				value_type sum = precision_[i][j];
				for (std::size_t k = 0; k < j; ++k) {
					sum -= lower[i][k] * lower[j][k];
				}
				if (i == j) {
					if (sum <= tol) {
						throw std::invalid_argument("Mahalanobis requires a symmetric positive definite matrix");
					}
					lower[i][j] = std::sqrt(sum);
				} else {
					lower[i][j] = sum / lower[j][j];
				}
			}
		}
	}
};

/**
 * Angular metric on finite unit vectors.
 *
 * Formula:
 *   d(x, y) = arccos(<x, y>)
 *
 * This is the geodesic metric on the unit sphere. The implementation rejects
 * zero vectors and vectors outside the configured unit-norm tolerance rather
 * than exposing the raw cosine complement as a metric.
 */
template <typename V = double> class Angular {
  public:
	using value_type = V;
	using distance_type = value_type;

	static_assert(std::is_floating_point<V>::value, "Angular requires a floating-point value type");

	explicit Angular(value_type unit_tolerance = value_type(1e-6)) : unit_tolerance_(unit_tolerance)
	{
		validate_tolerance();
	}

	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type
	{
		detail::require_aligned_vector_inputs(a, b, "Angular");
		validate_tolerance();
		if (a.empty()) {
			throw std::invalid_argument("Angular requires non-empty unit vectors");
		}
		const auto norms = dot_and_norms(a, b, "Angular");
		require_unit_norm(norms.norm_a, "Angular");
		require_unit_norm(norms.norm_b, "Angular");
		const auto cosine = std::max(value_type(-1), std::min(value_type(1), norms.dot));
		return std::acos(cosine);
	}

	auto unit_tolerance() const -> value_type { return unit_tolerance_; }

  private:
	struct dot_norms {
		value_type dot;
		value_type norm_a;
		value_type norm_b;
	};

	value_type unit_tolerance_;

	void validate_tolerance() const
	{
		if (!std::isfinite(unit_tolerance_) || unit_tolerance_ < value_type(0)) {
			throw std::invalid_argument("Angular requires a finite nonnegative unit tolerance");
		}
	}

	void require_unit_norm(value_type norm, const char *name) const
	{
		if (norm <= unit_tolerance_ || std::abs(norm - value_type(1)) > unit_tolerance_) {
			throw std::invalid_argument(std::string(name) + " requires unit-normalized nonzero vectors");
		}
	}

	template <typename Container> static auto dot_and_norms(const Container &a, const Container &b, const char *name)
		-> dot_norms
	{
		value_type dot = 0;
		value_type norm_a = 0;
		value_type norm_b = 0;
		for (std::size_t i = 0; i < a.size(); ++i) {
			const auto left = static_cast<value_type>(a[i]);
			const auto right = static_cast<value_type>(b[i]);
			if (!std::isfinite(left) || !std::isfinite(right)) {
				throw std::invalid_argument(std::string(name) + " requires finite vector entries");
			}
			dot += left * right;
			norm_a += left * left;
			norm_b += right * right;
		}
		return {dot, std::sqrt(norm_a), std::sqrt(norm_b)};
	}
};

/**
 * Chordal metric on finite unit vectors.
 *
 * Formula:
 *   d(x, y) = ||x - y||_2
 *
 * The same unit-vector gate as Angular is enforced, making this an ordinary
 * Euclidean metric restricted to the unit sphere.
 */
template <typename V = double> class Chordal {
  public:
	using value_type = V;
	using distance_type = value_type;

	static_assert(std::is_floating_point<V>::value, "Chordal requires a floating-point value type");

	explicit Chordal(value_type unit_tolerance = value_type(1e-6)) : unit_tolerance_(unit_tolerance)
	{
		validate_tolerance();
	}

	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type
	{
		detail::require_aligned_vector_inputs(a, b, "Chordal");
		validate_tolerance();
		if (a.empty()) {
			throw std::invalid_argument("Chordal requires non-empty unit vectors");
		}
		value_type norm_a = 0;
		value_type norm_b = 0;
		value_type sum = 0;
		for (std::size_t i = 0; i < a.size(); ++i) {
			const auto left = static_cast<value_type>(a[i]);
			const auto right = static_cast<value_type>(b[i]);
			if (!std::isfinite(left) || !std::isfinite(right)) {
				throw std::invalid_argument("Chordal requires finite vector entries");
			}
			norm_a += left * left;
			norm_b += right * right;
			const auto diff = left - right;
			sum += diff * diff;
		}
		require_unit_norm(std::sqrt(norm_a), "Chordal");
		require_unit_norm(std::sqrt(norm_b), "Chordal");
		return std::sqrt(sum);
	}

	auto unit_tolerance() const -> value_type { return unit_tolerance_; }

  private:
	value_type unit_tolerance_;

	void validate_tolerance() const
	{
		if (!std::isfinite(unit_tolerance_) || unit_tolerance_ < value_type(0)) {
			throw std::invalid_argument("Chordal requires a finite nonnegative unit tolerance");
		}
	}

	void require_unit_norm(value_type norm, const char *name) const
	{
		if (norm <= unit_tolerance_ || std::abs(norm - value_type(1)) > unit_tolerance_) {
			throw std::invalid_argument(std::string(name) + " requires unit-normalized nonzero vectors");
		}
	}
};

} // namespace mtrc

namespace mtrc::core {

template <typename V> struct metric_traits<::mtrc::WeightedMinkowski<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::WeightedMinkowski<V> &metric) -> std::string
	{
		std::string key = "mtrc::WeightedMinkowski:p=" + std::to_string(metric.exponent()) +
						  ":n=" + std::to_string(metric.weights().size());
		char buffer[64];
		for (const auto value : metric.weights()) {
			std::snprintf(buffer, sizeof(buffer), "%.17g", static_cast<double>(value));
			key += ":";
			key += buffer;
		}
		return key;
	}
};

template <typename V> struct metric_traits<::mtrc::Mahalanobis<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::Mahalanobis<V> &metric) -> std::string
	{
		std::string key = "mtrc::Mahalanobis:n=" + std::to_string(metric.precision_matrix().size());
		char buffer[64];
		for (const auto &row : metric.precision_matrix()) {
			key += ":";
			for (const auto value : row) {
				std::snprintf(buffer, sizeof(buffer), "%.17g", static_cast<double>(value));
				key += buffer;
				key += ",";
			}
		}
		return key;
	}
};

template <typename V> struct metric_traits<::mtrc::Angular<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::Angular<V> &metric) -> std::string
	{
		return std::string("mtrc::Angular:unit_tolerance=") + std::to_string(metric.unit_tolerance());
	}
};

template <typename V> struct metric_traits<::mtrc::Chordal<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::Chordal<V> &metric) -> std::string
	{
		return std::string("mtrc::Chordal:unit_tolerance=") + std::to_string(metric.unit_tolerance());
	}
};

} // namespace mtrc::core

#endif
