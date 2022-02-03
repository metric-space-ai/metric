/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019  Panda Team
*/
#ifndef _METRIC_SPACE_MATRIX_CPP
#define _METRIC_SPACE_MATRIX_CPP
#include "matrix.hpp"
#include <stdexcept>
#include <type_traits>

namespace metric {

template <typename RecType, typename Metric>
auto Matrix<RecType, Metric>::operator()(size_t i, size_t j) const -> distType
{
	check_index(i);
	check_index(j);
	if (i < j)
		return D_(i, j);
	return D_(j, i);
}

template <typename RecType, typename Metric> auto Matrix<RecType, Metric>::operator[](size_t index) const -> RecType
{
	return data_[index];
}

template <typename RecType, typename Metric> auto Matrix<RecType, Metric>::size() const -> std::size_t
{
	return data_.size();
}

template <typename RecType, typename Metric> auto Matrix<RecType, Metric>::insert(const RecType &item) -> std::size_t
{
	std::size_t old_size = data_.size();
	D_.resize(old_size + 1, old_size + 1, true);
	for (std::size_t i = 0; i < old_size; i++) {
		D_.insert(i, old_size, metric_(data_[i], item));
	}
	data_.push_back(item);
	return data_.size() - 1;
}

template <typename RecType, typename Metric>
template <typename Container, typename>
auto Matrix<RecType, Metric>::insert(const Container &items) -> std::vector<std::size_t>
{
	std::vector<std::size_t> ids;
	ids.reserve(items.size());
	for (auto &i : items) {
		auto id = insert(i);
		ids.push_back(id);
	}
	return ids;
}

template <typename RecType, typename Metric>
auto Matrix<RecType, Metric>::insert_if(const RecType &item, distType treshold) -> std::pair<std::size_t, bool>
{
	if (size() == 0) {
		return std::pair{insert(item), true};
	}
	std::pair<std::size_t, distType> nnid = nn_(item);
	if (nnid.second <= treshold)
		return std::pair{0, false};

	return std::pair{insert(item), true};
}

template <typename RecType, typename Metric>
template <typename Container, typename>
auto Matrix<RecType, Metric>::insert_if(const Container &items, distType treshold)
	-> std::vector<std::pair<std::size_t, bool>>
{
	std::vector<std::pair<std::size_t, bool>> ids{};
	ids.reserve(items.size());
	for (auto &i : items) {
		ids.push_back(insert_if(i, treshold));
	}
	return ids;
}

template <typename RecType, typename Metric> auto Matrix<RecType, Metric>::erase(std::size_t index) -> bool
{
	check_index(index);
	auto rows = D_.rows();
	if (index != rows - 1) {
		auto src = blaze::submatrix(D_, index + 1, index + 1, rows - index - 1, rows - index - 1);
		auto dst = blaze::submatrix(D_, index, index, rows - index - 1, rows - index - 1);
		dst = src;
		if (index != 0) {
			// auto src1 = blaze::submatrix(D_, 0, index + 1, id, rows - index - 1);
			// auto dst1 = blaze::submatrix(D_, 0, index, index, rows - index - 1);
			auto src1 = blaze::submatrix(D_, 0, index + 1, index, rows - index - 1);
			auto dst1 = blaze::submatrix(D_, 0, index, index, rows - index - 1);
			dst1 = src1;
		}
	}
	D_.resize(rows - 1, rows - 1, true);
	remove_data(index);
	return true;
}

template <typename RecType, typename Metric> void Matrix<RecType, Metric>::set(std::size_t index, const RecType &p)
{
	check_index(index);
	std::size_t old_size = D_.rows();
	for (std::size_t i = 0; i < old_size; i++) {
		if (i < index) {
			D_(i, index) = metric_(data_[i], p);
		} else if (i > index) {
			D_(index, i) = metric_(data_[i], p);
		}
	}
	data_[index] = p;
}

template <typename RecType, typename Metric> auto Matrix<RecType, Metric>::nn(const RecType &p) const -> std::size_t
{
	return nn_(p).first;
}

template <typename RecType, typename Metric>
auto Matrix<RecType, Metric>::knn(const RecType &query, unsigned k) const
	-> std::vector<std::pair<std::size_t, distType>>
{
	auto nnp = nn_(query);
	std::size_t nn_index = nnp.first;
	std::vector<std::pair<std::size_t, distType>> result;
	result.reserve(k);
	for (std::size_t i = 0; i < D_.columns(); ++i) {
		std::pair<std::size_t, distType> temp{i, metric_(query, data_[i])};

		auto ins = std::upper_bound(result.begin(), result.end(), temp,
									[](auto lhs, auto rhs) { return lhs.second < rhs.second; });

		result.insert(ins, temp);
		if (result.size() > k) {
			result.pop_back();
		}
	}
	return result;
}

template <typename RecType, typename Metric>
auto Matrix<RecType, Metric>::rnn(const RecType &query, distType range) const
	-> std::vector<std::pair<std::size_t, distType>>
{
	std::unordered_map<std::size_t, distType> metric_cache;
	auto nnp = nn_(query, metric_cache);
	std::size_t nn_index = nnp.first;
	std::vector<std::pair<std::size_t, distType>> result;
	if (nnp.second > range)
		return result;

	for (std::size_t idx = 0; idx < D_.columns(); idx++) {
		distType dist = metric_(query, data_[idx]);
		if (dist > range)
			continue;
		std::pair<std::size_t, distType> temp{idx, dist};
		auto ins = std::upper_bound(result.begin(), result.end(), temp,
									[](auto lhs, auto rhs) { return lhs.second < rhs.second; });
		result.insert(ins, temp);
	}
	return result;
}

template <typename RecType, typename Metric>
auto Matrix<RecType, Metric>::nn_(const RecType &p) const -> std::pair<std::size_t, distType>
{
	// brute force first nearest neighbour
	std::size_t nn_index = 0;
	distType min_dist = std::numeric_limits<distType>::max();
	for (std::size_t i = 0; i < data_.size(); i++) {
		auto dist = metric_(p, data_[i]);
		if (dist < min_dist) {
			min_dist = dist;
			nn_index = i;
		}
	}
	return std::pair{nn_index, min_dist};
}

template <typename RecType, typename Metric>
auto Matrix<RecType, Metric>::nn_(const RecType &p, std::unordered_map<std::size_t, distType> &metric_cache) const
	-> std::pair<std::size_t, distType>
{
	// brute force first nearest neighbour
	std::size_t nn_index = 0;
	distType min_dist = std::numeric_limits<distType>::max();
	for (std::size_t i = 0; i < data_.size(); i++) {
		auto dist = metric_(p, data_[i]);
		metric_cache[i] = dist;
		if (dist < min_dist) {
			min_dist = dist;
			nn_index = i;
		}
	}
	return std::pair{nn_index, min_dist};
}

} // namespace metric
#endif
