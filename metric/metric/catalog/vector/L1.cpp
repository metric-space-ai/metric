/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include "L1.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace mtrc {

template <typename V>
template <typename Container>
auto Sorensen<V>::operator()(const Container &a, const Container &b) const ->
	typename std::enable_if<!std::is_same<Container, V>::value, distance_type>::type
{
	detail::require_aligned_vector_inputs(a, b, "Sorensen");
	distance_type numerator = 0;
	distance_type denominator = 0;

	for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
		numerator += std::abs(*it1 - *it2);
		denominator += std::abs(*it1 + *it2);
	}

	return denominator == distance_type(0) ? distance_type(0) : numerator / denominator;
}

template <typename V>
auto Sorensen<V>::operator()(const mtrc::numeric::CompressedVector<V> &a,
							 const mtrc::numeric::CompressedVector<V> &b) const -> distance_type
{
	detail::require_aligned_vector_inputs(a, b, "Sorensen");
	distance_type numerator = 0;
	distance_type denominator = 0;

	for (std::size_t i = 0; i < a.size(); ++i) {
		numerator += std::abs(a[i] - b[i]);
		denominator += std::abs(a[i] + b[i]);
	}

	return denominator == distance_type(0) ? distance_type(0) : numerator / denominator;
}

template <typename Value>
double sorensen(const mtrc::numeric::CompressedVector<Value> &a, const mtrc::numeric::CompressedVector<Value> &b)
{
	return Sorensen<Value>{}(a, b);
}

template <typename V>
template <typename Container>
auto Hassanat<V>::operator()(const Container &A, const Container &B) const -> distance_type
{
	detail::require_aligned_vector_inputs(A, B, "Hassanat");
	value_type sum = 0;
	for (auto it1 = A.begin(), it2 = B.begin(); it1 != A.end() && it2 != B.end(); ++it1, ++it2) {
		const auto min_value = std::min(*it1, *it2);
		const auto max_value = std::max(*it1, *it2);
		if (min_value >= 0) {
			sum += 1 - (1 + min_value) / (1 + max_value);
		} else {
			sum += 1 - (1 + min_value + min_value) / (1 + max_value + min_value);
		}
	}
	return sum;
}

template <typename V>
template <typename Container>
auto Ruzicka<V>::operator()(const Container &A, const Container &B) const -> distance_type
{
	static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
	detail::require_aligned_vector_inputs(A, B, "Ruzicka");
	value_type min_sum = 0;
	value_type max_sum = 0;
	for (auto it1 = A.begin(), it2 = B.begin(); it1 != A.end() && it2 != B.end(); ++it1, ++it2) {
		const auto left = static_cast<value_type>(*it1);
		const auto right = static_cast<value_type>(*it2);
		if (!std::isfinite(left) || !std::isfinite(right) || left < value_type(0) || right < value_type(0)) {
			throw std::invalid_argument("Ruzicka requires finite nonnegative vector entries");
		}
		min_sum += std::min(left, right);
		max_sum += std::max(left, right);
	}
	if (max_sum == value_type(0)) {
		return value_type(0);
	}
	return value_type(1) - min_sum / max_sum;
}

} // namespace mtrc
