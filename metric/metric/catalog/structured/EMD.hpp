/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_K_STRUCTURED_EMD_HPP
#define _METRIC_DISTANCE_K_STRUCTURED_EMD_HPP
#include <cstddef>
#include <cstdio>
#include <limits>
#include <string>
#include <vector>

#include <metric/core/metric_traits.hpp>

namespace mtrc {

/**
 * @class EMD
 *
 * @brief Earth mover's distance (permissive compatibility route).
 *
 * This class is the historical, permissive Earth-Mover surface. It accepts an
 * arbitrary ground-cost matrix (which need not be a metric), supports unbalanced
 * masses through an extra-mass penalty, and lazily materialises a default cost
 * matrix on first use, mutating its own state inside a `const` call. None of
 * these behaviours preserve the metric axioms in general, so `EMD` is exposed as
 * `metric_law::distance` and must not be routed into metric-only algorithms.
 *
 * For the strict true-metric route (equal nonnegative mass over an admitted
 * metric ground cost) use @ref Wasserstein instead.
 */
template <typename V> class EMD {
  public:
	using value_type = V;
	using distance_type = value_type;

	explicit EMD() {}
	/**
	 * @brief Construct a new EMD object with cost matrix
	 *
	 * @param C_ cost matrix
	 */
	explicit EMD(std::vector<std::vector<value_type>> &&C_) : C(C_), is_C_initialized(true) {}

	/**
	 * @brief Construct a new EMD object
	 *
	 * @param rows, cols size of cost matrix
	 * @param extra_mass_penalty_
	 * @param F_
	 */
	EMD(std::size_t rows, std::size_t cols, const value_type &extra_mass_penalty_ = -1,
		std::vector<std::vector<value_type>> *F_ = nullptr)
		: C(default_ground_matrix(rows, cols)), extra_mass_penalty(extra_mass_penalty_), F(F_), is_C_initialized(true)
	{
	}

	/**
	 * @brief Construct a new EMD object
	 *
	 * @param C_  cost matrix
	 * @param extra_mass_penalty_
	 * @param F_
	 */
	EMD(const std::vector<std::vector<value_type>> &C_, const value_type &extra_mass_penalty_ = -1,
		std::vector<std::vector<value_type>> *F_ = nullptr)
		: C(C_), extra_mass_penalty(extra_mass_penalty_), F(F_), is_C_initialized(true)
	{
	}

	/**
	 * @brief Calculate EMD distance between Pc and Qc
	 *
	 * @tparam Container
	 * @param Pc
	 * @param Qc
	 * @return
	 */
	template <typename Container> distance_type operator()(const Container &Pc, const Container &Qc) const;

	EMD(EMD &&) = default;
	EMD(const EMD &) = default;
	EMD &operator=(const EMD &) = default;
	EMD &operator=(EMD &&) = default;

  private:
	mutable std::vector<std::vector<value_type>> C;
	value_type extra_mass_penalty = -1;
	std::vector<std::vector<value_type>> *F = nullptr;
	mutable bool is_C_initialized = false;

	std::vector<std::vector<value_type>> default_ground_matrix(std::size_t rows, std::size_t cols) const;
};

/**
 * @class Wasserstein
 *
 * @brief Strict 1-Wasserstein (Earth-Mover) true metric.
 *
 * `Wasserstein<V>` is the admitted, strict-metric split of @ref EMD. It computes
 * the optimal transport cost between two finite measures over a fixed ground
 * cost and is a true metric exactly when:
 *
 *  - the ground cost @p C is itself a metric on the bin indices
 *    (`C[i][i] == 0`, `C[i][j] > 0` for `i != j`, symmetric, and
 *    triangle-respecting), and
 *  - both inputs carry equal, nonnegative, finite mass.
 *
 * These conditions are enforced as hard guards: the constructor rejects any
 * ground cost that is not an admitted metric, and `operator()` rejects measures
 * that are misaligned, negative, non-finite, or of unequal total mass. Under
 * these guards the result satisfies non-negativity, identity of indiscernibles,
 * symmetry, and the triangle inequality, so the trait is `metric_law::metric`.
 *
 * The object is immutable after construction and therefore thread-safe; it does
 * not apply any extra-mass penalty (none is needed once equal mass is required).
 *
 * Domain and cost notes:
 *  - `V` must be a floating-point type (enforced by a `static_assert`). Integer
 *    count histograms should be converted to `double`/`float` first.
 *  - Equal mass is checked with a relative tolerance; equal zero-mass measures
 *    (and the single-bin case) return exactly `0`.
 *  - Construction validates the ground metric in `O(n^3)` (the triangle check)
 *    and the cache key serializes the full `n*n` matrix, so prefer small or line
 *    ground costs; an explicit dense cost for a large 2-D grid is expensive.
 */
template <typename V> class Wasserstein {
  public:
	using value_type = V;
	using distance_type = value_type;

	static_assert(std::is_floating_point<V>::value, "Wasserstein requires a floating-point value type");

	/**
	 * @brief Construct a strict Wasserstein metric over an admitted ground cost.
	 *
	 * @param ground_cost            square metric ground-cost matrix
	 * @param mass_relative_tolerance relative tolerance for the equal-mass guard
	 * @param ground_tolerance       absolute tolerance for the ground-metric
	 *                               checks; a negative value selects an automatic
	 *                               scale-relative tolerance
	 * @throws std::invalid_argument if @p ground_cost is not an admitted metric
	 *         or a tolerance is invalid.
	 */
	explicit Wasserstein(std::vector<std::vector<value_type>> ground_cost,
						 value_type mass_relative_tolerance = value_type(1e-6),
						 value_type ground_tolerance = value_type(-1));

	/**
	 * @brief Build a Wasserstein metric with the canonical line ground cost
	 *        `C[i][j] = |i - j|`, an unambiguous true metric on a 1-D grid.
	 *
	 * The line variant is evaluated by the closed-form cumulative-mass formula
	 * instead of the generic transport core, avoiding avoidable numerical drift
	 * in near-tight triangle checks.
	 *
	 * @param bins number of histogram bins (>= 1)
	 * @param mass_relative_tolerance relative tolerance for the equal-mass guard
	 * @throws std::invalid_argument if @p bins is zero.
	 */
	static Wasserstein on_line(std::size_t bins, value_type mass_relative_tolerance = value_type(1e-6));

	/**
	 * @brief Compute the 1-Wasserstein distance between measures @p P and @p Q.
	 *
	 * @throws std::invalid_argument if the measures are misaligned with the
	 *         ground cost, contain negative or non-finite mass, or carry unequal
	 *         total mass.
	 */
	template <typename Container> distance_type operator()(const Container &P, const Container &Q) const;

	auto size() const -> std::size_t { return C_.size(); }
	auto ground_cost() const -> const std::vector<std::vector<value_type>> & { return C_; }
	auto mass_relative_tolerance() const -> value_type { return mass_rtol_; }
	auto ground_tolerance() const -> value_type { return ground_tol_; }

	Wasserstein(const Wasserstein &) = default;
	Wasserstein(Wasserstein &&) = default;
	Wasserstein &operator=(const Wasserstein &) = default;
	Wasserstein &operator=(Wasserstein &&) = default;

	  private:
		Wasserstein(std::vector<std::vector<value_type>> ground_cost, value_type mass_relative_tolerance,
					value_type ground_tolerance, bool canonical_line_ground);

		std::vector<std::vector<value_type>> C_;
		value_type mass_rtol_;
		value_type ground_tol_;
		bool canonical_line_ground_;
	};

} // namespace mtrc

namespace mtrc::core {

// EMD is the permissive Earth-Mover compatibility route: arbitrary (possibly
// non-metric) ground cost, unbalanced mass via the extra-mass penalty, and
// mutable lazy state inside a const call. It is therefore not a true metric and
// is not safe to evaluate concurrently when default-constructed.
template <typename V> struct metric_traits<::mtrc::EMD<V>> {
	static constexpr auto law = metric_law::distance;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = false;
};

// Wasserstein is the strict 1-Wasserstein (EMD) true metric: equal nonnegative
// mass over an admitted metric ground cost, immutable after construction.
template <typename V> struct metric_traits<::mtrc::Wasserstein<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::Wasserstein<V> &metric) -> std::string
	{
		// Serialize each ground-cost entry at full round-trip precision so that
		// matrices differing only in low-order digits do not alias to one key.
		std::string key = "mtrc::Wasserstein:n=" + std::to_string(metric.size());
		char buffer[64];
		for (const auto &row : metric.ground_cost()) {
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

} // namespace mtrc::core

#include "EMD.cpp"

#endif // Header Guard
