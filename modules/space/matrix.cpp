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

template <typename recType, typename Metric>
auto Matrix<recType, Metric>::operator()(size_t i, size_t j) const -> distType
{
    return D_(i, j);
}

template <typename recType, typename Metric>
recType Matrix<recType, Metric>::operator[](size_t id) const
{
    return (data_(id));
}

template <typename recType, typename Metric>
size_t Matrix<recType, Metric>::size() const
{
    return data_.size();
}

template <typename recType, typename Metric>
std::size_t Matrix<recType, Metric>::insert(const recType& item)
{
    std::size_t old_size = D_.rows();
    D_.resize(old_size + 1, true);
    for (std::size_t i = 0; i < old_size; i++) {
        D_(i, old_size) = metric_(data_[i], item);
    }
    data_.push_back(item);
    return old_size;
}

template <typename recType, typename Metric>
template <typename Container,
          typename>
          // = std::enable_if<
          //     std::is_same<recType,
          //                  typename std::decay<decltype(std::declval<Container>().operator[](0))>::type>
          //     ::value>>
std::vector<std::size_t>
Matrix<recType, Metric>::insert(const Container& items)
{
    std::vector<std::size_t> ids;
    ids.reserve(items.size());
    for (auto& i : items) {
        auto id = insert(i);
        ids.push_back(id);
    }
    return ids;
}

// unimplemented stuff
template <typename recType, typename Metric>
std::pair<std::size_t, bool> Matrix<recType, Metric>::insert_if(const recType& p, distType threshold)
{
    //    throw std::runtime_error("not implemented yet");
    return std::pair { 0, false };
}
template <typename recType, typename Metric>
template <typename Container>
std::vector<std::pair<std::size_t, bool>> Matrix<recType, Metric>::insert_if(const Container& p, distType threshold)
{
    //    throw std::runtime_error("not implemented yet");
    return std::vector<std::pair<std::size_t, bool>> {};
}

template <typename recType, typename Metric>
bool Matrix<recType, Metric>::erase(std::size_t id)
{
    //    throw std::runtime_error("not implemented yet");
    return false;
}

template <typename recType, typename Metric>
void Matrix<recType, Metric>::set(std::size_t id, const recType& p)
{
    //    throw std::runtime_error("not implemented yet");
}

template <typename recType, typename Metric>
std::size_t Matrix<recType, Metric>::nn(const recType& p) const
{
    //    throw std::runtime_error("not implemented yet");
    return 0;
}

template <typename recType, typename Metric>
auto Matrix<recType, Metric>::knn(const recType& p, unsigned k) const -> std::vector<std::pair<std::size_t, distType>>
{
    //    throw std::runtime_error("not implemented yet");
    return std::vector<std::pair<std::size_t, distType>> {};
}

template <typename recType, typename Metric>
auto Matrix<recType, Metric>::rnn(const recType& p, distType distance) const
    -> std::vector<std::pair<std::size_t, distType>>
{
    //    throw std::runtime_error("not implemented yet");
    return std::vector<std::pair<std::size_t, distType>> {};
}

}  // namespace metric
#endif
