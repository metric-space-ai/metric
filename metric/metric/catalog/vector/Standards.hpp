/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_K_RELATED_STANDARDS_HPP
#define _METRIC_DISTANCE_K_RELATED_STANDARDS_HPP

#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/metric_traits.hpp>
#include <metric/metric/detail/vector_input.hpp>

namespace mtrc {

/**
 * @class Euclidean
 *
 * @brief Euclidean (L2) Metric
 */
template <typename V = double> struct Euclidean {
	using value_type = V;
	using distance_type = value_type;

	explicit Euclidean() = default;

	/**
	 * @brief Calculate Euclidean distance in R^n
	 *
	 * @param a first vector
	 * @param b second vector
	 * @return Euclidean distance between a and b
	 */
	template <typename Container>
	typename std::enable_if<!std::is_same<Container, V>::value, distance_type>::type
	operator()(const Container &a, const Container &b) const;

	/**
	 * @brief Calculate Euclidean distance in R
	 *
	 * @param a first value
	 * @param b second value
	 * @return Euclidean distance between a and b
	 */

	distance_type operator()(const V &a, const V &b) const;

	/**
	 * @brief Calculate Euclidean distance for Numeric input
	 *
	 * @param a first value
	 * @param b second value
	 * @return Euclidean distance between a and b
	 */
	template <template <typename, bool> class Container, typename ValueType,
			  bool F> // detect Numeric object by signature
	double operator()(const Container<ValueType, F> &a, const Container<ValueType, F> &b) const;
};

} // namespace mtrc

namespace mtrc::core {

template <typename V> struct metric_traits<::mtrc::Euclidean<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;
};

} // namespace mtrc::core

namespace mtrc {

/**
 * @class Manhattan
 *
 * @brief Manhattan/Cityblock (L1) Metric
 *
 */
template <typename V = double> struct Manhattan {
	using value_type = V;
	using distance_type = value_type;

	explicit Manhattan() = default;

	/**
	 * @brief Calculate Manhattan distance in R^n
	 *
	 * @param a first vector
	 * @param b second vector
	 * @return Manhattan distance between a and b
	 */

	template <typename Container> distance_type operator()(const Container &a, const Container &b) const;
};

/**
 * @class P_norm
 *
 * @brief Minkowski (L general) Metric
 *
 */
template <typename V = double> struct P_norm {
	using value_type = V;
	using distance_type = value_type;

	// P_norm() = default;
	/**
	 * @brief Construct a new P_norm object
	 *
	 * @param p_
	 */
	explicit P_norm(const value_type &p = 1) : p_(p)
	{
		static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
		if (!std::isfinite(p_) || p_ < value_type(1)) {
			throw std::invalid_argument("P_norm requires a finite exponent p >= 1");
		}
	}

	/**
	 * @brief calculate Minkowski distance
	 *
	 * @param a first vector
	 * @param b second vector
	 * @return Minkowski distance between a and b
	 */
	template <typename Container> distance_type operator()(const Container &a, const Container &b) const;

	auto exponent() const -> value_type { return p_; }

  private:
	value_type p_ = 1;
};

/**
 * @class Euclidean_threshold
 *
 * @brief Minkowski Metric (L... / P_Norm)
 *
 */
template <typename V = double> struct Euclidean_thresholded {
	using value_type = V;
	using distance_type = value_type;

	explicit Euclidean_thresholded() = default;

	/**
	 * @brief Construct a new Euclidean_thresholded object
	 *
	 * @param thres_
	 * @param factor_
	 */
	Euclidean_thresholded(value_type thres_, value_type factor_) : thres(thres_), factor(factor_) {}

	/**
	 * @brief
	 *
	 * @param a
	 * @param b
	 * @return
	 */
	template <typename Container> distance_type operator()(const Container &a, const Container &b) const;

	value_type thres = 1000.0;
	value_type factor = 3000.0;
};

template <typename V = double> struct Euclidean_hard_clipped {
	using value_type = V;
	using distance_type = value_type;

	value_type max_distance_;
	value_type scal_;
	explicit Euclidean_hard_clipped() = default;

	/**
	 * @brief Construct a new Euclidean_hard_clipped object
	 *
	 * @param max_distance
	 * @param scal
	 */
	Euclidean_hard_clipped(value_type max_distance = 100, value_type scal = 1)
		: max_distance_(max_distance), scal_(scal)
	{
	}

	/**
	 * @brief
	 *
	 * @param a
	 * @param b
	 * @return
	 */
	template <typename Container> distance_type operator()(const Container &a, const Container &b) const;
};

template <typename V = double> struct Euclidean_soft_clipped {
	using value_type = V;
	using distance_type = value_type;

	value_type max_distance_;
	value_type scal_;
	value_type thresh_;
	value_type F_;
	value_type x_;
	value_type T_;
	value_type y_;

	explicit Euclidean_soft_clipped() = default;

	/**
	 * @brief Construct a new Euclidean_hard_clipped object
	 *
	 * @param max_distance
	 * @param scal
	 * @param threshold
	 */
	Euclidean_soft_clipped(value_type max_distance = 100, value_type scal = 1, value_type thresh = 0.8)
		: max_distance_(max_distance), scal_(scal), thresh_(thresh)
	{
		F_ = (value_type(1) - thresh_) * max_distance;
		x_ = thresh_ * max_distance / scal;
		T_ = F_ / scal_;
		y_ = thresh_ * max_distance;
	}

	/**
	 * @brief
	 *
	 * @param a
	 * @param b
	 * @return
	 */
	template <typename Container> distance_type operator()(const Container &a, const Container &b) const;
};

/**
 * @class Euclidean_standardized
 *
 * @brief Standardized Euclidean metric `d(a, b) = || (a - b) / sigma ||_2`.
 *
 * Admitted metric domain
 * ----------------------
 * The admitted record is an aligned, finite real vector. The distance is the
 * Euclidean norm of the per-coordinate displacement after dividing by a fitted
 * positive scale `sigma`. Because the per-coordinate centering `mean` appears in
 * both terms it cancels exactly, so the distance depends only on `sigma`; the
 * `mean` member is kept for introspection and backward compatibility.
 *
 * When every `sigma[i]` is finite and strictly positive the map
 * `T(x)_i = x_i / sigma[i]` is an injective linear reparametrization, so the
 * standardized distance is the metric induced by the Euclidean norm through `T`:
 * it is non-negative, symmetric, satisfies the triangle inequality, and is zero
 * exactly on equal records (identity of indiscernibles). It is therefore
 * admitted as `metric_law::metric`.
 *
 * Parameter gate
 * --------------
 * A zero scale collapses a coordinate and destroys identity of indiscernibles
 * (and divides by zero), so every fitted scale must be finite and strictly
 * positive. The gate is enforced when fitting, when constructing from an
 * explicit `(mean, sigma)`, and on every evaluation (the scale vector is a public
 * member and could be mutated into a non-metric state). See
 * docs/metrics/metric-admission.md for the admission record.
 *
 * Numerical note
 * --------------
 * `V` must be a floating-point type (enforced by a `static_assert`); the admitted
 * record domain is finite real vectors. As with the bare `Euclidean`/`Manhattan`
 * norm metrics, the admission is a real-arithmetic statement: extreme finite
 * inputs or an extreme (but positive) scale can still overflow to `+inf` or
 * underflow under IEEE-754. Such results stay symmetric and triangle-respecting,
 * so they do not break metric routing, but they are a finite-precision boundary,
 * not part of the mathematical claim.
 */
template <typename V = double> struct Euclidean_standardized {
	using value_type = V;
	using distance_type = value_type;

	static_assert(std::is_floating_point<V>::value, "Euclidean_standardized requires a floating-point value type");

	std::vector<value_type> mean;
	std::vector<value_type> sigma;

	explicit Euclidean_standardized() = default;

	/**
	 * @brief Fit the standardized metric from aligned training records.
	 *
	 * The per-coordinate mean and population standard deviation are estimated
	 * from @p A. Every fitted scale must be finite and strictly positive, so a
	 * constant feature or a single training record (which both yield a zero
	 * scale) is rejected.
	 *
	 * @throws std::invalid_argument on empty/unaligned training data or a
	 *         non-positive fitted scale.
	 */
	template <typename Container> Euclidean_standardized(const Container &A);

	/**
	 * @brief Construct directly from an explicit centering @p mean_ and positive
	 *        scale @p sigma_.
	 *
	 * @throws std::invalid_argument if the dimensions differ or any scale is not
	 *         finite and strictly positive.
	 */
	Euclidean_standardized(std::vector<value_type> mean_, std::vector<value_type> sigma_)
		: mean(std::move(mean_)), sigma(std::move(sigma_))
	{
		validate_scales();
	}

	/**
	 * @brief Standardized Euclidean distance `|| (a - b) / sigma ||_2`.
	 *
	 * @throws std::invalid_argument on an unfitted/invalid scale, a dimension
	 *         mismatch, or non-finite inputs.
	 */
	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type;

	/// Enforce the admitted positive-scale gate (every sigma finite and > 0).
	void validate_scales() const;
};

/**
 * @class Manhattan_standardized
 *
 * @brief Standardized Manhattan metric `d(a, b) = sum_i |a_i - b_i| / sigma[i]`.
 *
 * This is the L1 analogue of @ref Euclidean_standardized: the displacement is
 * rescaled per coordinate by a fitted positive `sigma` and the centering `mean`
 * cancels. With every scale finite and strictly positive the rescaling is an
 * injective linear reparametrization, so the result is the metric induced by the
 * L1 norm and is admitted as `metric_law::metric`. The same positive-scale gate
 * is enforced on fit, on explicit construction, and on every evaluation. `V` must
 * be a floating-point type; the same IEEE-754 overflow/underflow boundary noted
 * for @ref Euclidean_standardized applies.
 */
template <typename V = double> struct Manhattan_standardized {
	using value_type = V;
	using distance_type = value_type;

	static_assert(std::is_floating_point<V>::value, "Manhattan_standardized requires a floating-point value type");

	std::vector<value_type> mean;
	std::vector<value_type> sigma;

	explicit Manhattan_standardized() = default;

	/**
	 * @brief Fit the standardized metric from aligned training records.
	 *
	 * @throws std::invalid_argument on empty/unaligned training data or a
	 *         non-positive fitted scale.
	 */
	template <typename Container> Manhattan_standardized(const Container &A);

	/**
	 * @brief Construct directly from an explicit centering @p mean_ and positive
	 *        scale @p sigma_.
	 *
	 * @throws std::invalid_argument if the dimensions differ or any scale is not
	 *         finite and strictly positive.
	 */
	Manhattan_standardized(std::vector<value_type> mean_, std::vector<value_type> sigma_)
		: mean(std::move(mean_)), sigma(std::move(sigma_))
	{
		validate_scales();
	}

	/**
	 * @brief Standardized Manhattan distance `sum_i |a_i - b_i| / sigma[i]`.
	 *
	 * @throws std::invalid_argument on an unfitted/invalid scale, a dimension
	 *         mismatch, or non-finite inputs.
	 */
	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type;

	/// Enforce the admitted positive-scale gate (every sigma finite and > 0).
	void validate_scales() const;
};

/**
 * @class Cosine
 *
 * @brief Cosine similarity
 *
 */

template <typename V = double> struct Cosine {
	using value_type = V;
	using distance_type = value_type;

	/**
	 * @brief calculate cosine similariy between two non-zero vector
	 *
	 * @param a first vector
	 * @param b second vector
	 * @return cosine similarity between a and b
	 */
	template <typename Container> distance_type operator()(const Container &a, const Container &b) const;
};

template <typename V = double> struct Weierstrass {
	using value_type = V;
	using distance_type = value_type;

	/**
	 * @brief calculate Weierstrass distance between two non-zero vector
	 *
	 * @param a first vector
	 * @param b second vector
	 * @return Weierstrass distance between a and b
	 */
	template <typename Container> distance_type operator()(const Container &a, const Container &b) const;
};

/**
 * @class CosineInverted
 *
 * @brief CosineInverted similarity. Means 1 - Cosine.
 *
 */

template <typename V = double> struct CosineInverted {
	using value_type = V;
	using distance_type = value_type;

	/**
	 * @brief calculate cosine similariy between two non-zero vector
	 *
	 * @param a first vector
	 * @param b second vector
	 * @return cosine similarity between a and b
	 */
	template <typename Container> distance_type operator()(const Container &a, const Container &b) const;
};

/**
 * @class Chebyshev
 *
 * @brief Chebyshev metric
 *
 */
template <typename V = double> struct Chebyshev {
	using value_type = V;
	using distance_type = value_type;

	explicit Chebyshev() = default;

	/**
	 * @brief calculate chebyshev metric
	 *
	 * @param lhs first container
	 * @param rhs second container
	 * @return Chebtshev distance between lhs and rhs
	 */
	template <typename Container> distance_type operator()(const Container &lhs, const Container &rhs) const;
};

} // namespace mtrc

namespace mtrc::core {

template <typename V> struct metric_traits<::mtrc::Manhattan<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;
};

template <typename V> struct metric_traits<::mtrc::P_norm<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::P_norm<V> &metric) -> std::string
	{
		return std::string("mtrc::P_norm:p=") + std::to_string(metric.exponent());
	}
};

template <typename V> struct metric_traits<::mtrc::Chebyshev<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;
};

// Standardized vector metrics are admitted true metrics when every fitted scale
// is finite and strictly positive (an injective linear reparametrization of the
// underlying L2/L1 norm). The gate is enforced on construction and on every
// evaluation. The distance depends only on `sigma` (the centering `mean`
// cancels), so the cache key serializes `sigma` at full round-trip precision.
template <typename V> struct metric_traits<::mtrc::Euclidean_standardized<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::Euclidean_standardized<V> &metric) -> std::string
	{
		std::string key = "mtrc::Euclidean_standardized:n=" + std::to_string(metric.sigma.size());
		char buffer[64];
		for (const auto value : metric.sigma) {
			std::snprintf(buffer, sizeof(buffer), "%.17g", static_cast<double>(value));
			key += ":";
			key += buffer;
		}
		return key;
	}
};

template <typename V> struct metric_traits<::mtrc::Manhattan_standardized<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::Manhattan_standardized<V> &metric) -> std::string
	{
		std::string key = "mtrc::Manhattan_standardized:n=" + std::to_string(metric.sigma.size());
		char buffer[64];
		for (const auto value : metric.sigma) {
			std::snprintf(buffer, sizeof(buffer), "%.17g", static_cast<double>(value));
			key += ":";
			key += buffer;
		}
		return key;
	}
};

} // namespace mtrc::core

#include "Standards.cpp"

#endif // Header Guard
