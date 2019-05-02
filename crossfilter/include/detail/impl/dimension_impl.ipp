/*This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2018 Dmitry Vinokurov */

#include <numeric>
#include <unordered_map>
#include "../../crossfilter.hpp"
//#define USE_STD_SORT
#ifndef USE_STD_SORT
#include "../../detail/impl/dual_pivot_sort2.hpp"
#endif

namespace cross {
namespace impl {
template<typename V, typename T, typename I, typename H>
inline
dimension_impl<V, T, I, H>::dimension_impl(cross::impl::filter_impl<T,H> *cf, std::size_t offset, int bit_num,
                                           std::function<field_type_t(const record_type_t &)> getter_)
    : crossfilter(cf),
    getter(getter_) {

  dimension_offset = offset;
  dimension_bit_index = bit_num;
  slot_add =  [this](std::size_t index, data_iterator begin, data_iterator end) {
                this->add(index,begin, end);
              };

  slot_remove = [this] (const std::vector<int64_t> &re_index) {
                  this->remove_data(re_index);
                };
  slot_post_add = [this](std::size_t new_data_size) {
                    add_signal(new_values, new_indexes, old_data_size, new_data_size);
                    new_values.clear();
                  };


  connection_add = crossfilter->connect_add_slot(slot_add);
  connection_remove = crossfilter->connect_remove_slot(slot_remove);
  connection_post_add = crossfilter->connect_post_add_slot(slot_post_add);

  refilter = [](const std::vector<value_type_t> &val) {
               return std::make_tuple<std::size_t, std::size_t>(0, val.size());
             };
}

template<typename V, typename T, typename I, typename H>
inline
void  dimension_impl<V, T, I, H>::clear() {
  iterables_index_count.clear();
  iterables_empty_rows.clear();
  low = high = 0;
  old_data_size = 0;
  values.clear();
  indexes.clear();
  new_values.clear();
  new_indexes.clear();
  added.clear();
  removed.clear();
  refilter_function_flag = false;
}

template<typename V, typename T, typename I, typename H>
inline
void  dimension_impl<V, T, I, H>::dispose() {
  dispose_dimension_signal();
  connection_add.disconnect();
  connection_remove.disconnect();
  filter_all();
}

template<typename V, typename T, typename I, typename H>
inline
bool dimension_impl<V, T, I, H>::zero_except(std::size_t index) {
  return crossfilter->zero_except(index,
                                  this->dimension_offset,
                                  this->dimension_bit_index);
}


template<typename V, typename T, typename I, typename H>
inline
bool  dimension_impl<V, T, I, H>::only_except(std::size_t index, std::size_t offset, int bit_index) {

  return crossfilter->only_except(index,
                                  this->dimension_offset, this->dimension_bit_index, offset, bit_index);
}

template<typename V, typename T, typename I, typename H>
inline
typename dimension_impl<V, T, I, H>::connection_type_t
dimension_impl<V, T, I, H>::connect_filter_slot(std::function<void(std::size_t, int,
                                                                   const std::vector<std::size_t> &,
                                                                   const std::vector<std::size_t> &,
                                                                   bool)> slot) {
  return crossfilter->connect_filter_slot(slot);
}


template<typename V, typename T, typename I, typename H>
inline
typename dimension_impl<V, T, I, H>::connection_type_t
dimension_impl<V, T, I, H>::connect_add_slot(std::function<void(const std::vector<value_type_t>&,
                                                                const std::vector<std::size_t> &,
                                                                std::size_t, std::size_t)> slot) {
  return add_signal.connect(slot);
}


template<typename V, typename T, typename I, typename H>
inline
typename dimension_impl<V, T, I, H>::connection_type_t
dimension_impl<V, T, I, H>::connect_remove_slot(std::function<void(const std::vector<int64_t> &)> slot) {
  return remove_signal.connect(slot);
}

template<typename V, typename T, typename I, typename H>
inline
typename dimension_impl<V, T, I, H>::connection_type_t
dimension_impl<V, T, I, H>::connect_dispose_slot(std::function<void()> slot) {
  return dispose_dimension_signal.connect(slot);
}

template<typename V, typename T, typename I, typename H>
template<bool Enable>
inline
std::enable_if_t<dimension_impl<V,T,I,H>::isIterable && Enable>
dimension_impl<V, T, I, H>::add(std::size_t insert_point, data_iterator begin, data_iterator end) {
  // auto logger = spdlog::get("console");
  // if(logger) logger->info("dimension_iter _add start");
  new_values.clear();
  new_indexes.clear();
  // if(logger) logger->info("dimension _add start");
  std::size_t new_data_size = std::distance(begin, end);
  old_data_size = crossfilter->size() - new_data_size;
  std::vector<std::size_t> unsorted_index;
  std::size_t l = 0;
  std::vector<std::size_t> new_iterables_index_count(new_data_size); // map indexes at new_data to size of conteiner
  auto empty_rows = 0;
  for (std::size_t index1 = 0; index1 < new_data_size; index1++) {
    auto k = getter(*(begin + index1));

    if (k.empty()) {
      new_iterables_index_count[index1] = 0;
      iterables_empty_rows.push_back(index1 + insert_point);
      l++;
      continue;
    }
    auto ks = k.size();
    new_iterables_index_count[index1] = ks;
    for (std::size_t j = 0; j < ks; ++j) {
      new_values.push_back(k[j]);
      unsorted_index.push_back(index1 + insert_point);
      l++;
    }
  }
  std::vector<std::size_t> new_index(new_values.size());


  std::iota(new_index.begin(), new_index.end(), 0);
  // if(logger)   logger->info("dimension_iter _add prepare");
#ifndef USE_STD_SORT
  DualPivotsort2::quicksort<std::size_t, value_type_t>(new_index, 0, new_index.size(),
                                                       [this](auto r) { return new_values[r];});
#else
  std::sort(new_index.begin(), new_index.end(),
            [this](auto lhs, auto rhs) {
              return new_values[lhs] < new_values[rhs];
            });
#endif
  //  if(logger)   logger->info("dimension_iter _add sort {}", new_index.size());
  std::vector<value_type_t> tmp_values;
  tmp_values.reserve(new_index.size());
  std::vector<std::size_t> tmp_index;
  tmp_index.reserve(new_index.size());

  for (auto &i : new_index) {
    tmp_values.push_back(new_values[i]);
    tmp_index.push_back(unsorted_index[i]);
  }
  std::swap(tmp_values, new_values);
  std::swap(tmp_index, new_indexes);

  //  if(logger)   logger->info("dimension_iter add refilter");
  auto bounds = refilter(new_values);
  auto lo1 = std::get<0>(bounds);
  auto hi1 = std::get<1>(bounds);

  std::unordered_map<std::size_t, std::size_t> new_filter_status;
  if(refilter_function_flag) {
    for(std::size_t i = 0; i < new_values.size(); i++) {
      if(!refilter_function(new_values[i]))
        new_filter_status[new_indexes[i]-insert_point] += 1;
    }
  } else {
    for (std::size_t i = 0; i < lo1; ++i) {
      new_filter_status[new_indexes[i]-insert_point] += 1;
    }
    auto nvs = new_values.size();

    for (std::size_t i = hi1; i < nvs; ++i) {
      new_filter_status[new_indexes[i]-insert_point] += 1;
    }
  }

  for(auto & f : new_filter_status) {
    // if all elements of iterable data field filtered out - set filter bit
    if(f.second == new_iterables_index_count[f.first])
      crossfilter->set_bit_for_dimension(f.first+insert_point, this->dimension_offset, this->dimension_bit_index);

  }
  new_data_size = l; //(isIterable) ? l : new_data_size;
  //  if(logger)   logger->info("dimension_iter _add filter");
  if (values.empty()) {
    values = new_values;
    indexes = new_indexes;
    std::swap(iterables_index_count, new_iterables_index_count);
    low = lo1;
    high = hi1;
    return;
  }

  std::vector<value_type_t> old_values;
  std::swap(values, old_values);
  std::vector<std::size_t> old_indexes;
  std::swap(old_indexes, indexes);

  values.resize(old_values.size() + new_values.size());
  indexes.resize(old_values.size() + new_values.size());
  if (isIterable) {
    iterables_index_count.insert(iterables_index_count.begin()+insert_point, new_iterables_index_count.begin(), new_iterables_index_count.end());
  }
  // merge oldValues and new_values
  std::size_t index5 = 0;
  std::size_t i1 = 0;
  std::size_t i2 = 0;
  auto nvs = new_values.size();
  auto ovs = old_values.size();
  //  if(logger)   logger->info("dimension_iter _add prepare merge");
  for (; i1 < ovs && i2 < nvs; ++index5) {
    if (new_values[i2] < old_values[i1]) {
      values[index5] = new_values[i2];
      indexes[index5] = new_indexes[i2];
      i2++;
    } else {
      values[index5] = old_values[i1];
      auto oi = old_indexes[i1];
      indexes[index5] = (oi >= insert_point) ? oi + new_data_size + empty_rows: oi;
      // indexes[index5] = old_indexes[i1];  
      i1++;
    }
  }
  for (; i1 < ovs; i1++, index5++) {
    values[index5] = old_values[i1];
    auto oi = old_indexes[i1];
    indexes[index5] = (oi >= insert_point) ? oi + new_data_size : oi;
    //    indexes[index5] = old_indexes[i1];
  }

  for (; i2 < nvs; i2++, index5++) {
    values[index5] = new_values[i2];
    indexes[index5] = new_indexes[i2];
  }
  //  if(logger)   logger->info("dimension_iter _add prepare merge done");
  // Bisect again to recompute low and high.
  bounds = refilter(values);
  low = std::get<0>(bounds);
  high = std::get<1>(bounds);
  //  if(logger)   logger->info("dimension _add prepare refilter");
}

template<typename V, typename T, typename I, typename H>
template<bool Enable>
inline
std::enable_if_t<!dimension_impl<V,T,I,H>::isIterable && Enable>
dimension_impl<V, T, I, H>::add(std::size_t insert_point, data_iterator begin, data_iterator end) {
  //  auto logger = spdlog::get("console");
  //  if(logger) logger->info("dimension _add start");

  old_data_size = values.size();

  std::size_t new_data_size = std::distance(begin, end);
  new_indexes.clear();
  new_indexes.reserve(new_data_size);

  new_values.clear();
  new_values.reserve(new_indexes.size());
  std::size_t ip = insert_point;
  std::vector<std::pair<std::size_t,value_type_t>> new_values2;
  new_values.reserve(new_indexes.size());
  for(auto p = begin; p != end; ++p,++ip) {
    new_values2.push_back(std::make_pair(ip,getter(*p)));
  }

  //  if(logger)   logger->info("dimension _add prepare");

#ifndef USE_STD_SORT
  DualPivotsort2::quicksort<std::pair<std::size_t,value_type_t>, value_type_t>(new_values2, 0, new_values2.size(),
                                                       [](auto & r) { return r.second;});

#else
  std::sort(new_values2.begin(), new_values2.end(),
            [](auto & lhs, auto& rhs) {
              return lhs.second < rhs.second;
            });
#endif
  //  if(logger)   logger->info("dimension _add sort {}", new_values2.size());
  for (auto &i : new_values2) {
    new_values.push_back(i.second);
    new_indexes.push_back(i.first);
  }
  //  if(logger)   logger->info("dimension add refilter");
  auto bounds = refilter(new_values);
  auto lo1 = std::get<0>(bounds);
  auto hi1 = std::get<1>(bounds);
  if (refilter_function) {
    for (std::size_t index2 = 0; index2 < new_data_size; ++index2) {
      if (!refilter_function(new_values[index2])) {
        crossfilter->set_bit_for_dimension(new_indexes[index2],
                                           this->dimension_offset, this->dimension_bit_index);
      }
    }
  } else {
    for (std::size_t i = 0; i < lo1; ++i) {
      crossfilter->set_bit_for_dimension(new_indexes[i], this->dimension_offset, this->dimension_bit_index);
    }
    for (std::size_t i = hi1; i < new_data_size; ++i) {
      crossfilter->set_bit_for_dimension(new_indexes[i], this->dimension_offset, this->dimension_bit_index);
    }
  }
  //  if(logger)   logger->info("dimension _add filter");
  if (values.empty()) {
    values = new_values;
    indexes = new_indexes;
    low = lo1;
    high = hi1;
    return;
  }
  std::vector<value_type_t> old_values;
  std::swap(values, old_values);
  std::vector<std::size_t> old_indexes;
  std::swap(old_indexes, indexes);
  values.resize(old_values.size() + new_values.size());
  indexes.resize(old_values.size() + new_values.size());

  // merge oldValues and new_values
  std::size_t index5 = 0;
  std::size_t i1 = 0;
  std::size_t i2 = 0;
  auto ovs = old_values.size();
  auto nvs = new_values.size();
  //  if(logger)   logger->info("dimension _add prepare merge");

  for (; i1 < ovs && i2 < nvs; ++index5) {
    if (new_values[i2] < old_values[i1]) {
      values[index5] = new_values[i2];
      indexes[index5] = new_indexes[i2];
      i2++;
    } else {
      values[index5] = old_values[i1];
      auto oi = old_indexes[i1];
      indexes[index5] = (oi >= insert_point) ? oi + new_data_size : oi;
      i1++;
    }
  }
  for (; i1 < ovs; i1++, index5++) {
    values[index5] = old_values[i1];
    auto oi = old_indexes[i1];
    indexes[index5] = (oi >= insert_point) ? oi + new_data_size : oi;
  }

  for (; i2 < nvs; i2++, index5++) {
    values[index5] = new_values[i2];
    indexes[index5] = new_indexes[i2];
  }
  //  if(logger)   logger->info("dimension _add prepare merge done");
  // Bisect again to recompute low and high.
  bounds = refilter(values);
  low = std::get<0>(bounds);
  high = std::get<1>(bounds);
  //  if(logger)   logger->info("dimension _add prepare refilter");
}

template<typename V, typename T, typename I, typename H>
inline
void  dimension_impl<V, T, I, H>::remove_data_iterable(const std::vector<int64_t> &re_index) {
  if (isIterable) {
    std::size_t i1 = 0;

    for (std::size_t i0 = 0; i0 < iterables_empty_rows.size(); i0++) {
      if (re_index[iterables_empty_rows[i0]] != REMOVED_INDEX) {
        iterables_empty_rows[i1] = re_index[iterables_empty_rows[i0]];
        i1++;
      }
    }
    iterables_empty_rows.resize(i1);
    i1 = 0;
    for (std::size_t i0 = 0; i0 < iterables_index_count.size(); i0++) {
      if (re_index[i0] != REMOVED_INDEX) {
        iterables_index_count[i1] = iterables_index_count[i0];
        i1++;
      }
    }
    iterables_index_count.resize(i1);
  }


  // Rewrite our index, overwriting removed values
  std::size_t n0 = values.size();
  std::size_t j = 0;
  for (std::size_t i = 0; i < n0; ++i) {
    auto cindex = indexes[i];
    if (re_index[cindex] != REMOVED_INDEX) {
      if (i != j) {
        values[j] = values[i];
        indexes[j] = re_index[cindex];
      }
      ++j;
    }
  }
  values.resize(j);
  indexes.resize(j);

  // Bisect again to recompute lo0 and hi0.
  auto bounds = refilter(values);
  low = std::get<0>(bounds);
  high = std::get<1>(bounds);

  remove_signal(re_index);
}

template<typename V, typename T, typename I, typename H>
inline
void  dimension_impl<V, T, I, H>::remove_data(const std::vector<int64_t> &re_index) {
  if (isIterable) {
    remove_data_iterable(re_index);
    return;
  }

  // Rewrite our index, overwriting removed values
  std::size_t n0 = values.size();
  std::size_t j = 0;
  std::vector<int64_t> n_re_index(n0);
  for(std::size_t i = 0; i < n0; i++) {
    if(re_index[indexes[i]] == REMOVED_INDEX)
      n_re_index[i] = -1;
    else
      n_re_index[i] = indexes[i];
  }
  for (std::size_t i = 0; i < n0; ++i) {
    auto old_data_index = indexes[i];
    if (re_index[old_data_index] != REMOVED_INDEX) {
      if (i != j)
        values[j] = values[i];
      indexes[j] = re_index[old_data_index];
      ++j;
    }
  }
  values.resize(j);
  indexes.resize(j);
  // Bisect again to recompute lo0 and hi0.
  auto bounds = refilter(values);
  low = std::get<0>(bounds);
  high = std::get<1>(bounds);

  remove_signal(re_index);
}

template<typename V, typename T, typename I, typename H>
inline
std::tuple<std::size_t, std::size_t>
dimension_impl<V, T, I, H>::refilter_range(const value_type_t & left,
                                           const value_type_t & right,
                                           const std::vector<value_type_t> & data) {
  auto filter_low = std::lower_bound(data.begin(), data.end(), left);
  auto filter_high = std::lower_bound(data.begin(), data.end(), right);
  std::size_t filter_low_index = std::distance(data.begin(), filter_low);
  std::size_t filter_high_index = std::distance(data.begin(), filter_high);
  return std::make_tuple<std::size_t, std::size_t>(
      std::move(filter_low_index), std::move(filter_high_index));
}

template<typename V, typename T, typename I, typename H>
inline
std::tuple<std::size_t, std::size_t>
dimension_impl<V, T, I, H>::refilter_exact(const value_type_t & value,
                                           const std::vector<value_type_t> & data) {
  auto filters = std::equal_range(data.begin(), data.end(), value);
  std::size_t filter_low_index = std::distance(data.begin(), filters.first);
  std::size_t filter_high_index = std::distance(data.begin(), filters.second);

  return std::make_tuple<std::size_t, std::size_t>(
      std::move(filter_low_index), std::move(filter_high_index));
}

template<typename V, typename T, typename I, typename H>
template<bool Enable>
inline
std::enable_if_t<dimension_impl<V,T,I,H>::isIterable && Enable>
dimension_impl<V, T, I, H>::do_filter(std::size_t filter_low_index, std::size_t filter_high_index) {
  std::unordered_map<std::size_t, std::size_t> filter_status_added;
  std::unordered_map<std::size_t, std::size_t> filter_status_removed;
  // std::vector<std::size_t> added;
  // std::vector<std::size_t> removed;
  added.clear();
  removed.clear();
  
  filter_status_added.reserve((filter_high_index - filter_low_index)/2);
  filter_status_removed.reserve((filter_high_index - filter_low_index)/2);
  if(refilter_function_flag) {
    // reset previous filter
    refilter_function_flag = false;
    for(std::size_t i = 0; i < indexes.size(); i++) {
      if(i >= filter_low_index && i < filter_high_index) {
        if(crossfilter->get_bit_for_dimension_st(indexes[i],this->dimension_offset, this->dimension_bit_index)) {
          filter_status_added[indexes[i]] += 1;
        }
      } else {
        if(!crossfilter->get_bit_for_dimension_st(indexes[i],this->dimension_offset, this->dimension_bit_index)) {
          filter_status_removed[indexes[i]] += 1;
        }
      }
    }
  } else {

    if (filter_low_index < low) {
      for (auto i = filter_low_index; i < std::min(low, filter_high_index); ++i) {
        auto ind = indexes[i];
        auto & status = filter_status_added[ind];
        if(status == 0)  {
          if(crossfilter->get_bit_for_dimension_st(ind,this->dimension_offset, this->dimension_bit_index)) {
            status += 1;
          }
        } else {
          status += 1;
        }

      }
    } else if (filter_low_index > low) {
      for (auto i = low; i < std::min(filter_low_index, high); ++i) {
        filter_status_removed[indexes[i]] +=1;
      }
    }
    // Fast incremental update based on previous hi index.
    if (filter_high_index > high) {
      for (auto i = std::max(filter_low_index, high); i < filter_high_index; ++i) {
        auto ind = indexes[i];
        auto & status = filter_status_added[ind];
        if(status == 0)  {
          if(crossfilter->get_bit_for_dimension_st(indexes[i],this->dimension_offset, this->dimension_bit_index)) {
            status += 1;
          }
        } else {
          status += 1;
        }
      }
    } else if (filter_high_index < high) {
      for (auto i = std::max(low, filter_high_index); i < high; ++i) {
        filter_status_removed[indexes[i]] += 1;
      }
    }
  }
  if(filter_low_index == 0 && filter_high_index == values.size()) {
    for(auto i : iterables_empty_rows) {
      if(crossfilter->get_bit_for_dimension_st(i,this->dimension_offset,this->dimension_bit_index))
        filter_status_added[i] = 0;
    }
  } else {
    for(auto i : iterables_empty_rows) {
      if(!crossfilter->get_bit_for_dimension_st(i,this->dimension_offset,this->dimension_bit_index))
        filter_status_removed[i] = 0;
    }
  }
  added.reserve(filter_status_added.size()/2);
  removed.reserve(filter_status_removed.size()/2);
  for(auto f : filter_status_added) {
    if(!isIterable || f.second == iterables_index_count[f.first]) {
      crossfilter->reset_bit_for_dimension_st(f.first,this->dimension_offset,this->dimension_bit_index);
      added.push_back(f.first);
    }
  }

  for(auto f : filter_status_removed) {
    if(!isIterable || f.second == iterables_index_count[f.first]) {
      removed.push_back(f.first);
      crossfilter->set_bit_for_dimension_st(f.first,this->dimension_offset,this->dimension_bit_index);
    }
  }

  low = filter_low_index;
  high = filter_high_index;
  crossfilter->emit_filter_signal(this->dimension_offset, this->dimension_bit_index, added, removed);
}

template<typename V, typename T, typename I, typename H>
inline void dimension_impl<V, T, I, H>::filter1(std::vector<std::size_t> & added, std::vector<std::size_t>& removed, std::size_t filter_low_index, std::size_t filter_high_index) {
  if (filter_low_index < low) {
    for (auto i = filter_low_index; i < std::min(low, filter_high_index); ++i) {
      auto ind = indexes[i];
      if(crossfilter->get_bit_for_dimension_st(ind,this->dimension_offset, this->dimension_bit_index)) {
        added.push_back(ind);
      }
    }
  } else if (filter_low_index > low) {
    for (auto i = low; i < std::min(filter_low_index, high); ++i) {
      removed.push_back(indexes[i]);
    }
  }
}
template<typename V, typename T, typename I, typename H>
inline void dimension_impl<V, T, I, H>::filter2(std::vector<std::size_t> & added, std::vector<std::size_t>& removed, std::size_t filter_low_index, std::size_t filter_high_index) {
  // Fast incremental update based on previous hi index.
  if (filter_high_index > high) {
    for (auto i = std::max(filter_low_index, high); i < filter_high_index; ++i) {
      auto ind = indexes[i];
      if(crossfilter->get_bit_for_dimension_st(ind,this->dimension_offset, this->dimension_bit_index)) {
        added.push_back(ind);
      }
    }
  } else if (filter_high_index < high) {
    for (auto i = std::max(low, filter_high_index); i < high; ++i) {
      removed.push_back(indexes[i]);
    }
  }
}


template<typename V, typename T, typename I, typename H>
template<bool Enable>
inline
std::enable_if_t<!dimension_impl<V,T,I,H>::isIterable && Enable>
dimension_impl<V, T, I, H>::do_filter(std::size_t filter_low_index, std::size_t filter_high_index) {
  // std::vector<std::size_t> added;
  // std::vector<std::size_t> removed;
  // auto logger = spdlog::get("console");
  // logger->info("do_filter_start");

  added.clear();
  removed.clear();

  added.reserve(indexes.size()/2);
  removed.reserve(indexes.size()/2);

  //  logger->info("do_filter_prepare");

  if(refilter_function_flag) {
    // reset previous filter
    refilter_function_flag = false;
    for(std::size_t i = 0; i < indexes.size(); i++) {
      if(i >= filter_low_index && i < filter_high_index) {
        if(crossfilter->get_bit_for_dimension_st(indexes[i],this->dimension_offset, this->dimension_bit_index)) {
          added.push_back(indexes[i]);
        }
      } else {
        if(!crossfilter->get_bit_for_dimension_st(indexes[i],this->dimension_offset, this->dimension_bit_index)) {
          removed.push_back(indexes[i]);
        }
      }
    }
  } else {
    filter1(added,removed,filter_low_index, filter_high_index);
    //    logger->info("do_filter_filter1");
    filter2(added,removed,filter_low_index, filter_high_index);
    //    logger->info("do_filter_filter2");
  }
  for(auto f : added) {
    crossfilter->reset_bit_for_dimension_st(f,this->dimension_offset,this->dimension_bit_index);
  }
  //  logger->info("do_filter_aded");
  for(auto f : removed) {
    crossfilter->set_bit_for_dimension_st(f,this->dimension_offset,this->dimension_bit_index);
  }
  //  logger->info("do_filter_removed");
  low = filter_low_index;
  high = filter_high_index;
  crossfilter->emit_filter_signal(this->dimension_offset, this->dimension_bit_index, added, removed);
  //  logger->info("do_filter_emit_signal");
}


template<typename V, typename T, typename I, typename H>
inline
void  dimension_impl<V, T, I, H>::filter_range(const value_type_t & left, const value_type_t &right) {
  //  refilterFunctionFlag = false;
  //  spdlog::get("console")->info("filter_range");
  refilter = [left, right, this](const std::vector<value_type_t> &val) {
               return this->refilter_range(left, right, val);
             };
  auto bounds = refilter(values);
  auto filter_low_index = std::get<0>(bounds);
  auto filter_high_index = std::get<1>(bounds);
  do_filter(filter_low_index, filter_high_index);
}

template<typename V, typename T, typename I, typename H>
inline
void  dimension_impl<V, T, I, H>::filter_exact(const value_type_t & value) {
  refilter = [value, this](const std::vector<value_type_t> &val) {
               return this->refilter_exact(value, val);
             };
  auto bounds = refilter(values);
  auto filter_low_index = std::get<0>(bounds);
  auto filter_high_index = std::get<1>(bounds);
  do_filter(filter_low_index, filter_high_index);
}

template<typename V, typename T, typename I, typename H>
inline
void  dimension_impl<V, T, I, H>::filter_all() {
  refilter = [](const std::vector<value_type_t> &val) {
               return std::make_tuple<std::size_t, std::size_t>(0, val.size());
             };

  do_filter(0, values.size());
}

template<typename V, typename T, typename I, typename H>
inline
void  dimension_impl<V, T, I, H>::filter_with_predicate(
    std::function<bool(const value_type_t&)> filter_func) {
  refilter = [](const std::vector<value_type_t> &val) {
               return std::make_tuple<std::size_t, std::size_t>(0, val.size());
             };
  if(!isIterable) {
    refilter_function = filter_func;
    refilter_function_flag = true;
    // std::vector<std::size_t> added;
    // std::vector<std::size_t> removed;
    added.clear();
    removed.clear();
    
    for (std::size_t i = 0; i < values.size(); i++) {
      if (crossfilter->get_bit_for_dimension(indexes[i], this->dimension_offset, this->dimension_bit_index)
          && filter_func(values[i])) {
        added.push_back(indexes[i]);
        crossfilter->reset_bit_for_dimension(indexes[i], this->dimension_offset, this->dimension_bit_index);
        continue;
      }
      if (!crossfilter->get_bit_for_dimension(indexes[i], this->dimension_offset, this->dimension_bit_index)
          && !filter_func(values[i])) {
        removed.push_back(indexes[i]);
        crossfilter->set_bit_for_dimension(indexes[i], this->dimension_offset, this->dimension_bit_index);
      }
    }
    low = 0;
    high = values.size();
    crossfilter->emit_filter_signal(this->dimension_offset, this->dimension_bit_index, added, removed);
  } else {
    std::unordered_map<std::size_t, std::size_t> filter_status_added;
    std::unordered_map<std::size_t, std::size_t> filter_status_removed;
    std::unordered_map<std::size_t, std::vector<std::size_t>> removed_indexes;
    std::unordered_map<std::size_t, std::vector<std::size_t>> added_indexes;
    // std::vector<std::size_t> added;
    // std::vector<std::size_t> removed;
    added.clear();
    removed.clear();
    
    for(std::size_t i = 0; i < values.size(); i++) {
      bool filter_status = crossfilter->get_bit_for_dimension(indexes[i], this->dimension_offset, this->dimension_bit_index);
      auto filter_test = filter_func(values[i]);
      if(filter_status == false) { // item is in filter
        if(!filter_test) {
          filter_status_removed[indexes[i]] += 1;
          removed_indexes[indexes[i]].push_back(i);
        }
      } else { // item is not in filter
        if(filter_test) {
          filter_status_added[indexes[i]] += 1;
          added_indexes[indexes[i]].push_back(i);
        }
      }
    }
    for(auto i : iterables_empty_rows) {
      if(!crossfilter->get_bit_for_dimension(i,this->dimension_offset,this->dimension_bit_index))
        filter_status_removed[i] = 0;
    }
    for(auto f : filter_status_added) {
      if(f.second != 0) {
        crossfilter->reset_bit_for_dimension(f.first,this->dimension_offset,this->dimension_bit_index);
        added.push_back(f.first);
      }
    }

    for(auto f : filter_status_removed) {
      if(f.second == iterables_index_count[f.first]) {
        removed.push_back(f.first);
        crossfilter->set_bit_for_dimension(f.first,this->dimension_offset,this->dimension_bit_index);
      }
    }
    refilter_function = filter_func;
    refilter_function_flag = true;
    low = 0;
    high = values.size();
    crossfilter->emit_filter_signal(this->dimension_offset, this->dimension_bit_index, added, removed);

  }
  //  crossfilter->trigger_on_change("filtered");
}

template<typename V, typename T, typename I, typename H>
inline
auto
dimension_impl<V, T, I, H>::bottom(int64_t k, int64_t bottom_offset) const noexcept ->std::vector<record_type_t> {
  if (low == high && low == crossfilter->size()  && iterables_empty_rows.empty())
    return std::vector<record_type_t>();
  std::vector<T> result;
  auto high_iter = indexes.begin();
  auto low_iter = indexes.begin();
  std::advance(high_iter, high);
  std::advance(low_iter, low);
  int64_t result_size = 0;
  for(auto p = iterables_empty_rows.begin(); p < iterables_empty_rows.end() && result_size < k; p++) {
    if(crossfilter->zero(*p)) {
      if (bottom_offset-- > 0)
        continue;
      result.push_back(crossfilter->get_raw(*p));
      result_size++;
    }
  }
  if (low_iter > high_iter)
    return result;
  for (; low_iter < high_iter && result_size < k; low_iter++) {
    if (crossfilter->zero(*low_iter)) {
      if (bottom_offset-- > 0)
        continue;
      result.push_back(crossfilter->get_raw(*low_iter));
      result_size++;
    }
  }
  return result;

}

template<typename V, typename T, typename I, typename H>
inline
auto
dimension_impl<V, T, I, H>::top(int64_t k, int64_t top_offset) const noexcept -> std::vector<record_type_t> {
  if (low == high && low == crossfilter->size() && iterables_empty_rows.empty())
    return std::vector<record_type_t>();
  std::vector<T> result;
  auto high_iter = indexes.begin();
  auto low_iter = indexes.begin();
  std::advance(high_iter, high - 1);
  std::advance(low_iter, low);
  int64_t result_size = 0;

  if  (high_iter >= low_iter) {
    for (; high_iter >= low_iter && result_size < k ; high_iter--) {
      if (crossfilter->zero(*high_iter)) {
        if (top_offset-- > 0)
          continue;
        result.push_back(crossfilter->get_raw(*high_iter));
        result_size++;
      }
    }
  }
  if(result_size < k) {
    for(auto p = iterables_empty_rows.begin(); p < iterables_empty_rows.end() && result_size < k; p++) {
      if(crossfilter->zero(*p)) {
        if (top_offset-- > 0)
          continue;
        result.push_back(crossfilter->get_raw(*p));
        result_size++;
      }
    }
  }
  return result;
}

template<typename V, typename T, typename I, typename H>
template <typename K, typename R>
inline
auto
dimension_impl<V, T, I, H>::feature(cross::dimension<V,T,I,H> *base,
                                    std::function<K(const value_type_t &)> key,
                                    std::function<R(R &, const record_type_t &, bool)> add_func_,  
                                    std::function<R(R &, const record_type_t &, bool)> remove_func_,
                                    std::function<R()> initial_func_) noexcept -> cross::feature<K, R, this_type_t,false> {
  cross::feature<K, R, this_type_t, false> g(base, key, add_func_, remove_func_, initial_func_);
  g.add(values, indexes, 0, crossfilter->size());
  return g;
}

template<typename V, typename T, typename I, typename H>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
inline
auto
dimension_impl<V, T, I, H>::feature_all(cross::dimension<V,T,I,H> *base,
                                        AddFunc add_func_,
                                        RemoveFunc remove_func_,
                                        InitialFunc initial_func_
                                        ) noexcept -> cross::feature<std::size_t, decltype(initial_func_()), this_type_t, true> {
  cross::feature<std::size_t, decltype(initial_func_()), this_type_t, true> g(base,
                                                                              [](const value_type_t &) { return std::size_t(0); },
                                                                              add_func_, remove_func_, initial_func_);
  g.add(values, indexes, 0, crossfilter->size());
  return g;
}

} //namespace impl
} //namespace cross
