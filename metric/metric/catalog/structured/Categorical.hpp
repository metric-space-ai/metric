/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef _METRIC_METRIC_CATALOG_STRUCTURED_CATEGORICAL_HPP
#define _METRIC_METRIC_CATALOG_STRUCTURED_CATEGORICAL_HPP

#include <cmath>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <metric/core/metric_traits.hpp>
#include <metric/metric/catalog/vector/L1.hpp>
#include <metric/metric/detail/vector_input.hpp>

namespace mtrc {

/**
 * Discrete metric over records with stable equality.
 *
 * Formula:
 *   d(x, y) = 0 if x == y, otherwise mismatch_cost
 *
 * The mismatch cost must be finite and strictly positive. Epsilon equality is
 * deliberately not used here because it would change the record identity law.
 */
template <typename V = double> class DiscreteMetric {
  public:
	using value_type = V;
	using distance_type = value_type;

	explicit DiscreteMetric(value_type mismatch_cost = value_type(1)) : mismatch_cost_(mismatch_cost)
	{
		validate_cost();
	}

	template <typename Record> auto operator()(const Record &a, const Record &b) const -> distance_type
	{
		validate_cost();
		return a == b ? value_type(0) : mismatch_cost_;
	}

	auto mismatch_cost() const -> value_type { return mismatch_cost_; }

  private:
	value_type mismatch_cost_;

	void validate_cost() const
	{
		if constexpr (std::is_floating_point<value_type>::value) {
			if (!std::isfinite(mismatch_cost_)) {
				throw std::invalid_argument("DiscreteMetric requires a finite mismatch cost");
			}
		}
		if (mismatch_cost_ <= value_type(0)) {
			throw std::invalid_argument("DiscreteMetric requires a positive mismatch cost");
		}
	}
};

/**
 * Hamming metric on equal-length aligned categorical records.
 *
 * Formula:
 *   d(x, y) = |{i : x_i != y_i}|
 *
 * Variable-length padded variants are intentionally not admitted here; callers
 * should use edit distance when insertions/deletions are part of the domain.
 */
struct Hamming {
	using distance_type = std::size_t;

	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type
	{
		detail::require_aligned_vector_inputs(a, b, "Hamming");
		distance_type distance = 0;
		auto left = a.begin();
		auto right = b.begin();
		for (; left != a.end(); ++left, ++right) {
			if (!(*left == *right)) {
				++distance;
			}
		}
		return distance;
	}
};

/**
 * Jaccard metric on finite set records.
 *
 * The input container is treated as a representation of a finite set: order and
 * duplicates are ignored by canonicalizing into `std::set`. The empty/empty
 * convention is fixed at distance 0.
 */
class Jaccard {
  public:
	using distance_type = double;

	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type
	{
		using value_type = typename Container::value_type;
		const std::set<value_type> left(a.begin(), a.end());
		const std::set<value_type> right(b.begin(), b.end());

		if (left.empty() && right.empty()) {
			return 0.0;
		}

		std::size_t intersection = 0;
		auto lit = left.begin();
		auto rit = right.begin();
		while (lit != left.end() && rit != right.end()) {
			if (*lit < *rit) {
				++lit;
			} else if (*rit < *lit) {
				++rit;
			} else {
				++intersection;
				++lit;
				++rit;
			}
		}

		const auto union_size = left.size() + right.size() - intersection;
		return 1.0 - static_cast<double>(intersection) / static_cast<double>(union_size);
	}
};

/**
 * Jaccard metric for aligned binary presence vectors.
 *
 * Entries must be exactly 0 or 1. This keeps identity tied to the binary vector
 * itself; arbitrary nonzero numeric weights should use Ruzicka/Tanimoto instead.
 */
class BinaryJaccard {
  public:
	using distance_type = double;

	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type
	{
		detail::require_aligned_vector_inputs(a, b, "BinaryJaccard");
		std::size_t intersection = 0;
		std::size_t union_size = 0;
		for (std::size_t i = 0; i < a.size(); ++i) {
			const double left = static_cast<double>(a[i]);
			const double right = static_cast<double>(b[i]);
			if (!std::isfinite(left) || !std::isfinite(right) ||
				!((left == 0.0 || left == 1.0) && (right == 0.0 || right == 1.0))) {
				throw std::invalid_argument("BinaryJaccard requires binary entries 0 or 1");
			}
			const bool left_present = left != 0.0;
			const bool right_present = right != 0.0;
			if (left_present && right_present) {
				++intersection;
			}
			if (left_present || right_present) {
				++union_size;
			}
		}
		if (union_size == 0) {
			return 0.0;
		}
		return 1.0 - static_cast<double>(intersection) / static_cast<double>(union_size);
	}
};

/**
 * Tanimoto distance for finite nonnegative aligned vectors.
 *
 * This admitted variant is the weighted-Jaccard/Ruzicka computation:
 *   d(x, y) = 1 - sum_i min(x_i, y_i) / sum_i max(x_i, y_i)
 *
 * It intentionally does not expose generic Tanimoto complements whose metric
 * law depends on a different formula or domain.
 */
template <typename V = double> struct Tanimoto {
	using value_type = V;
	using distance_type = value_type;

	template <typename Container> auto operator()(const Container &a, const Container &b) const -> distance_type
	{
		return Ruzicka<V>{}(a, b);
	}
};

} // namespace mtrc

namespace mtrc::core {

template <typename V> struct metric_traits<::mtrc::DiscreteMetric<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ::mtrc::DiscreteMetric<V> &metric) -> std::string
	{
		return std::string("mtrc::DiscreteMetric:mismatch_cost=") + std::to_string(metric.mismatch_cost());
	}
};

template <> struct metric_traits<::mtrc::Hamming> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::sequence;
	static constexpr bool thread_safe = true;
};

template <> struct metric_traits<::mtrc::Jaccard> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::structured;
	static constexpr bool thread_safe = true;
};

template <> struct metric_traits<::mtrc::BinaryJaccard> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;
};

template <typename V> struct metric_traits<::mtrc::Tanimoto<V>> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = true;
};

} // namespace mtrc::core

#endif
