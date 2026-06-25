/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/

#include "Standards.hpp"

#include <metric/numeric.hpp>

#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace mtrc {

template <typename V>
template <typename Container>
auto Euclidean<V>::operator()(const Container &a, const Container &b) const ->
	typename std::enable_if<!std::is_same<Container, V>::value, distance_type>::type
{
	// static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
	detail::require_aligned_vector_inputs(a, b, "Euclidean");
	distance_type sum = 0;
	for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
		sum += (*it1 - *it2) * (*it1 - *it2);
	}
	return std::sqrt(sum);
}

template <typename V> auto Euclidean<V>::operator()(const V &a, const V &b) const -> distance_type
{
	static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
	distance_type sum = 0;
	sum += (a - b) * (a - b);
	return std::sqrt(sum);
}

template <typename V>
template <template <typename, bool> class Container, typename ValueType, bool F> // detect Numeric object by signature
double Euclidean<V>::operator()(const Container<ValueType, F> &a, const Container<ValueType, F> &b) const
{
	return mtrc::numeric::norm(a - b);
}

template <typename V>
template <typename Container>
auto Euclidean_thresholded<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
	static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
	detail::require_aligned_vector_inputs(a, b, "Euclidean_thresholded");
	distance_type sum = 0;
	for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
		sum += (*it1 - *it2) * (*it1 - *it2);
	}
	return std::min(thres, value_type(factor * std::sqrt(sum)));
}

template <typename V>
template <typename Container>
auto Euclidean_hard_clipped<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
	static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
	detail::require_aligned_vector_inputs(a, b, "Euclidean_hard_clipped");
	distance_type sum = 0;
	for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
		sum += (*it1 - *it2) * (*it1 - *it2);
	}
	return std::min(max_distance_, value_type(scal_ * std::sqrt(sum)));
}

template <typename V>
template <typename Container>
auto Euclidean_soft_clipped<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
	static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
	detail::require_aligned_vector_inputs(a, b, "Euclidean_soft_clipped");
	distance_type sum = 0;
	for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
		sum += (*it1 - *it2) * (*it1 - *it2);
	}
	auto distance = std::sqrt(sum);
	if (distance > x_) {
		return F_ * (value_type(1) - std::exp((-distance + x_) / T_)) + y_;
	} else {
		return scal_ * distance;
	}
}

template <typename V> void Euclidean_standardized<V>::validate_scales() const
{
	if (sigma.empty()) {
		throw std::invalid_argument("Euclidean_standardized requires a calibrated positive scale (sigma)");
	}
	if (mean.size() != sigma.size()) {
		throw std::invalid_argument("Euclidean_standardized requires mean and sigma of equal dimension");
	}
	for (const auto s : sigma) {
		if (!std::isfinite(s) || s <= value_type(0)) {
			throw std::invalid_argument(
				"Euclidean_standardized requires every calibrated scale sigma to be finite and positive");
		}
	}
	for (const auto m : mean) {
		if (!std::isfinite(m)) {
			throw std::invalid_argument("Euclidean_standardized requires a finite calibrated mean");
		}
	}
}

template <typename V>
template <typename Container>
Euclidean_standardized<V>::Euclidean_standardized(const Container &A)
{
	const auto dimension = detail::aligned_vector_dimension(A, "Euclidean_standardized");
	mean.assign(dimension, value_type(0));
	sigma.assign(dimension, value_type(0));

	for (const auto &record : A) {
		if (record.size() != dimension) {
			throw std::invalid_argument("Euclidean_standardized requires aligned calibration records");
		}
		for (size_t i = 0; i < dimension; ++i) {
			mean[i] += record[i];
		}
	}
	for (size_t i = 0; i < mean.size(); ++i) {
		mean[i] /= value_type(A.size());
	}

	for (const auto &record : A) {
		for (size_t i = 0; i < dimension; ++i) {
			sigma[i] += std::pow(record[i] - mean[i], 2);
		}
	}
	for (size_t i = 0; i < sigma.size(); ++i) {
		sigma[i] = std::sqrt(sigma[i] / value_type(A.size()));
	}

	// A zero or non-finite calibrated scale (e.g. a constant coordinate or a
	// single calibration record) is not an admitted true metric, so reject it.
	validate_scales();
}

template <typename V>
template <typename Container>
auto Euclidean_standardized<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
	detail::require_aligned_vector_inputs(a, b, "Euclidean_standardized");
	validate_scales();
	if (a.size() != sigma.size()) {
		throw std::invalid_argument("Euclidean_standardized input dimension does not match calibrated dimension");
	}
	distance_type sum = 0;
	for (size_t i = 0; i < a.size(); ++i) {
		const auto da = static_cast<distance_type>(a[i]);
		const auto db = static_cast<distance_type>(b[i]);
		if (!std::isfinite(da) || !std::isfinite(db)) {
			throw std::invalid_argument("Euclidean_standardized requires finite vector entries");
		}
		// The centering `mean` cancels: ((a-mean) - (b-mean)) / sigma = (a-b)/sigma.
		const auto diff = (da - db) / sigma[i];
		sum += diff * diff;
	}
	return std::sqrt(sum);
}

template <typename V> void Manhattan_standardized<V>::validate_scales() const
{
	if (sigma.empty()) {
		throw std::invalid_argument("Manhattan_standardized requires a calibrated positive scale (sigma)");
	}
	if (mean.size() != sigma.size()) {
		throw std::invalid_argument("Manhattan_standardized requires mean and sigma of equal dimension");
	}
	for (const auto s : sigma) {
		if (!std::isfinite(s) || s <= value_type(0)) {
			throw std::invalid_argument(
				"Manhattan_standardized requires every calibrated scale sigma to be finite and positive");
		}
	}
	for (const auto m : mean) {
		if (!std::isfinite(m)) {
			throw std::invalid_argument("Manhattan_standardized requires a finite calibrated mean");
		}
	}
}

template <typename V>
template <typename Container>
Manhattan_standardized<V>::Manhattan_standardized(const Container &A)
{
	const auto dimension = detail::aligned_vector_dimension(A, "Manhattan_standardized");
	mean.assign(dimension, value_type(0));
	sigma.assign(dimension, value_type(0));

	for (const auto &record : A) {
		if (record.size() != dimension) {
			throw std::invalid_argument("Manhattan_standardized requires aligned calibration records");
		}
		for (size_t i = 0; i < dimension; ++i) {
			mean[i] += record[i];
		}
	}
	for (size_t i = 0; i < mean.size(); ++i) {
		mean[i] /= value_type(A.size());
	}

	for (const auto &record : A) {
		for (size_t i = 0; i < dimension; ++i) {
			sigma[i] += std::pow(record[i] - mean[i], 2);
		}
	}
	for (size_t i = 0; i < sigma.size(); ++i) {
		sigma[i] = std::sqrt(sigma[i] / value_type(A.size()));
	}

	// Reject a zero or non-finite calibrated scale before any metric use.
	validate_scales();
}

template <typename V>
template <typename Container>
auto Manhattan_standardized<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
	detail::require_aligned_vector_inputs(a, b, "Manhattan_standardized");
	validate_scales();
	if (a.size() != sigma.size()) {
		throw std::invalid_argument("Manhattan_standardized input dimension does not match calibrated dimension");
	}
	distance_type sum = 0;
	for (size_t i = 0; i < a.size(); ++i) {
		const auto da = static_cast<distance_type>(a[i]);
		const auto db = static_cast<distance_type>(b[i]);
		if (!std::isfinite(da) || !std::isfinite(db)) {
			throw std::invalid_argument("Manhattan_standardized requires finite vector entries");
		}
		// The centering `mean` cancels: ((a-mean) - (b-mean)) / sigma = (a-b)/sigma.
		sum += std::abs((da - db) / sigma[i]);
	}
	return sum;
}

template <typename V>
template <typename Container>
auto Manhattan<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
	static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
	detail::require_aligned_vector_inputs(a, b, "Manhattan");
	distance_type sum = 0;
	for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
		sum += std::abs(*it1 - *it2);
	}
	return sum;
}

template <typename V>
template <typename Container>
auto P_norm<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
	static_assert(std::is_floating_point<value_type>::value, "T must be a float type");
	detail::require_aligned_vector_inputs(a, b, "P_norm");
	distance_type sum = 0;
	for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
		sum += std::pow(std::abs(*it1 - *it2), p_);
	}
	return std::pow(sum, value_type(1) / p_);
}

template <typename V>
template <typename Container>
auto Cosine<V>::operator()(const Container &A, const Container &B) const -> distance_type
{
	detail::require_aligned_vector_inputs(A, B, "Cosine");
	value_type dot = 0, denom_a = 0, denom_b = 0;
	for (auto it1 = A.begin(), it2 = B.begin(); it1 != A.end() && it2 != B.end(); ++it1, ++it2) {
		dot += *it1 * *it2;
		denom_a += *it1 * *it1;
		denom_b += *it2 * *it2;
	}
	return std::acos(dot / (std::sqrt(denom_a) * std::sqrt(denom_b))) / M_PI;
}

template <typename V>
template <typename Container>
auto Weierstrass<V>::operator()(const Container &A, const Container &B) const -> distance_type
{
	detail::require_aligned_vector_inputs(A, B, "Weierstrass");
	value_type dot_ab = 0, dot_a = 0, dot_b = 0;
	for (auto it1 = A.begin(), it2 = B.begin(); it1 != A.end() && it2 != B.end(); ++it1, ++it2) {
		dot_ab += *it1 * *it2;
		dot_a += *it1 * *it1;
		dot_b += *it2 * *it2;
	}
	return std::acosh(std::sqrt(1 + dot_a) * std::sqrt(1 + dot_b) - dot_ab);
}

template <typename V>
template <typename Container>
auto CosineInverted<V>::operator()(const Container &A, const Container &B) const -> distance_type
{
	detail::require_aligned_vector_inputs(A, B, "CosineInverted");
	value_type dot = 0, denom_a = 0, denom_b = 0;
	for (auto it1 = A.begin(), it2 = B.begin(); it1 != A.end() && it2 != B.end(); ++it1, ++it2) {
		dot += *it1 * *it2;
		denom_a += *it1 * *it1;
		denom_b += *it2 * *it2;
	}
	return std::abs(1 - dot / (std::sqrt(denom_a) * std::sqrt(denom_b)));
}

template <typename V>
template <typename Container>
auto Chebyshev<V>::operator()(const Container &lhs, const Container &rhs) const -> distance_type
{
	detail::require_aligned_vector_inputs(lhs, rhs, "Chebyshev");
	distance_type res = 0;
	for (std::size_t i = 0; i < lhs.size(); i++) {
		auto m = std::abs(lhs[i] - rhs[i]);
		if (m > res)
			res = m;
	}
	return res;
}

} // namespace mtrc
