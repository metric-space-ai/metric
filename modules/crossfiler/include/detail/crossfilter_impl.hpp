/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef CROSSFILTER_IMPL_H_GUARD
#define CROSSFILTER_IMPL_H_GUARD

#include <vector>
#include <functional>
#include <iostream>
#include <limits>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include "../detail/signal_base.hpp"
#include "../detail/bitarray.hpp"
#include "../detail/utils.hpp"
#include "../detail/dimension.hpp"
#include "../detail/feature.hpp"
#include "../detail/thread_policy.hpp"

namespace cross {
template <typename, typename> struct filter;
namespace impl {

template<typename T, typename Hash> struct filter_impl {

 public:
  template <typename, typename, typename, typename> friend struct cross::dimension;
  template <typename , typename, typename,  bool> friend struct cross::impl::feature_impl;

  static constexpr std::size_t dimension_offset = std::numeric_limits<std::size_t>::max();
  static constexpr int dimension_bit_index = -1;
  //  mutable std::mutex mutex;
  mutable cross::thread_policy::mutex_type mutex;
  using record_type_t = T;
  using this_type_t = filter<T,Hash>;
  using value_type_t = T;

  using index_vec_t = std::vector<std::size_t>;
  using index_set_t = std::vector<std::size_t>;
  using data_iterator = typename std::vector<T>::const_iterator;

  template<typename F> using signal_type_t = MovableSignal<typename signals::signal<F>, signals::connection>;
  using connection_type_t = signals::connection;

  signal_type_t<void(std::size_t, int,
                     const index_set_t &, const index_set_t &, bool)>  filter_signal;

  signal_type_t<void(const std::vector<int64_t> &)> remove_signal;

  signal_type_t<void(const std::vector<value_type_t> &, const std::vector<std::size_t>&, std::size_t, std::size_t)> add_group_signal;
  signal_type_t<void()> dispose_signal;
  signal_type_t<void()> clear_signal;
  signal_type_t<void(std::size_t)>                  post_add_signal;
  signal_type_t<void(std::size_t, data_iterator, data_iterator)> add_signal;


  BitArray filters;
  std::vector<record_type_t> data;
  std::size_t data_size = 0;

  std::unordered_map<uint64_t,uint64_t> hash_table;
  Hash hash;

#ifdef CROSS_FILTER_USE_THREAD_POOL
  uint32_t thread_pool_size = 2;
  void set_thread_pool_size(uint32_t threads) {
    thread_pool_size = threads;
  }
#endif

  virtual ~filter_impl() {}
  filter_impl(Hash h):hash(h) {}
  filter_impl(filter_impl &&) = default;
  template<typename Iterator>
  filter_impl(Iterator begin, Iterator end, Hash h):hash(h) {
    data.assign(begin,end);
    data_size = data.size();
    filters.resize(data_size);
  }

  std::vector<T> bottom(std::size_t low, std::size_t high, int64_t k,
                        int64_t offset, const index_vec_t &indexes, const index_vec_t &empty);

  std::vector<T> top(std::size_t low, std::size_t high, int64_t k,
                     int64_t offset, const index_vec_t &indexes, const index_vec_t &empty);

  connection_type_t
  connect_filter_slot(std::function<void(std::size_t, int,
                                         const index_set_t &, const index_set_t &, bool)> slot);

  connection_type_t
  connect_remove_slot(std::function<void(const std::vector<int64_t> &)> slot);

  connection_type_t
  connect_add_slot(std::function<void(const std::vector<value_type_t> &, const std::vector<std::size_t> &,
                                      std::size_t, std::size_t)>);
  connection_type_t
  connect_dispose_slot(std::function<void()> slot);

  connection_type_t
  connect_post_add_slot(std::function<void(std::size_t)> slot);


  connection_type_t
  connect_add_slot(std::function<void(std::size_t, data_iterator, data_iterator)> slot);

  connection_type_t
  connect_clear_slot(std::function<void()> callback);

  virtual
  void
  trigger_on_change(cross::event event) = 0;

  void
  emit_filter_signal(std::size_t filter_offset, int filter_bit_num,
                     const index_set_t &added, const index_set_t &removed) {
    filter_signal(filter_offset, filter_bit_num, added, removed, false);
  }



  std::size_t size() const {
    return data.size();
  }

  std::vector<T> all() const {
    return data;
  } 

  const T & get_raw(std::size_t index) const {
    return data[index];
  }


  template <typename Iterator>
  filter_impl<T,Hash> & add(std::size_t index, Iterator first, Iterator last,  bool allow_duplicates);

  template<typename C>
  typename std::enable_if<!std::is_same<C, T>::value, filter_impl&>::type
  add(const C & new_data, bool allow_duplicates);

  filter_impl<T,Hash> & add(std::size_t pos, const T & new_data,  bool allow_duplicates);

  template<typename Iterator>
  filter_impl<T,Hash> & add(Iterator first, Iterator last,  bool allow_duplicates);

  template<bool B = true>
  typename std::enable_if<std::is_same<Hash,cross::trivial_hash<T>>::value && B, std::size_t>::type
  do_add_one(std::size_t pos, const T & new_data,  bool allow_duplicates);

  template<bool B = true>
  typename std::enable_if<!std::is_same<Hash,cross::trivial_hash<T>>::value && B, std::size_t>::type
  do_add_one(std::size_t pos, const T & new_data,  bool allow_duplicates);

  template<typename Iterator, bool B = true>
  typename std::enable_if<std::is_same<Hash,cross::trivial_hash<T>>::value && B, std::size_t>::type
  do_add_range(std::size_t pos, Iterator first, Iterator last,  bool allow_duplicates);

  template<typename Iterator, bool B = true>
  typename std::enable_if<!std::is_same<Hash,cross::trivial_hash<T>>::value && B, std::size_t>::type
  do_add_range(std::size_t pos, Iterator first, Iterator last,  bool allow_duplicates);

  template<typename... _Args> std::size_t emplace (std::size_t position, _Args &&... args);

  std::vector<uint8_t> & mask_for_dimension(std::vector<uint8_t> & mask) const;

  template<typename D>
  std::vector<uint8_t> & mask_for_dimension(std::vector<uint8_t> & mask,
                                            const D & dim) const;

  template<typename D, typename ...Ts>
  std::vector<uint8_t> & mask_for_dimension(std::vector<uint8_t> & mask,
                                            const D & dim,
                                            Ts&... dimensions) const;

  template<typename ...Ts>
  std::vector<T> all_filtered(Ts&... dimension) const;
  std::vector<T> all_filtered_except_mask(const std::vector<uint8_t> & mask) const;

  template<typename ...Ts>
  bool is_element_filtered(std::size_t index, Ts&... dimension) const;
  bool is_element_filtered_except_mask(std::size_t index, const std::vector<uint8_t> & mask) const;

  template<typename G>
  auto dimension(G getter) -> cross::dimension<decltype(getter(std::declval<record_type_t>())), T, cross::non_iterable, Hash>;

  template<typename V, typename G>
  auto iterable_dimension(G getter) -> cross::dimension<V, T, cross::iterable, Hash>;


  void flip_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                              int dimension_bit_index);

  void
  set_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                        int dimension_bit_index);
  void
  reset_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                          int dimension_bit_index);

  bool
  get_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                        int dimension_bit_index);

  void flip_bit_for_dimension_st(std::size_t index, std::size_t dimension_offset,
                              int dimension_bit_index);

  void
  set_bit_for_dimension_st(std::size_t index, std::size_t dimension_offset,
                        int dimension_bit_index);
  void
  reset_bit_for_dimension_st(std::size_t index, std::size_t dimension_offset,
                          int dimension_bit_index);

  bool
  get_bit_for_dimension_st(std::size_t index, std::size_t dimension_offset,
                        int dimension_bit_index);

  std::tuple<data_iterator, data_iterator>
  get_data_iterators_for_indexes(std::size_t low, std::size_t high);

  template <bool B = true>
  typename std::enable_if<!std::is_same<Hash,cross::trivial_hash<T>>::value && B, void>::type
  update_hash_on_remove(std::size_t index);

  template <bool B = true>
  typename std::enable_if<std::is_same<Hash,cross::trivial_hash<T>>::value && B, void>::type
  update_hash_on_remove(std::size_t index);

  template<typename P>
  void remove_data(P should_remove, std::size_t first, std::size_t last);
  //  void remove_data(std::function<bool(int)> should_remove, std::size_t first, std::size_t last);

  // removes all records matching the predicate (ignoring filters).
  template<typename P>
  void remove(P predicate);
  //  void remove(std::function<bool(const T&, int)> predicate);

  // Removes all records that match the current filters
  void remove();

  //clear all data;
  void clear();

  //remove range [first,last)
  void remove(std::size_t first, std::size_t last);

  bool
  zero_except(std::size_t index) {
    return filters.zero_except(index,
                               this->dimension_offset,
                               this->dimension_bit_index);
  }

  bool
  zero_except(std::size_t index, std::size_t offset, int bit_index) {
    return filters.zero_except(index, offset, bit_index);
  }

  bool
  only_except(std::size_t index, std::size_t offset, int bit_index) {
    return filters.only(index, offset, bit_index);
  }

  bool
  only_except(std::size_t index, std::size_t offset1, int bit_index1,
              std::size_t offset2, int bit_index2) {
    return filters.only_except(index, offset1, bit_index1, offset2, bit_index2);
  }


  bool zero(std::size_t index) {
    return filters.zero(index);
  }

  bool only(std::size_t index, std::size_t offset, std::size_t bit_index) {
    return filters.only(index, offset, bit_index);
  }

  std::size_t translate_index(std::size_t index) const {
    return index;
  }


  template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
  auto feature(
      cross::filter<T,Hash> * base,
      AddFunc  add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_) -> cross::feature<std::size_t,
                                                   decltype(initial_func_()), this_type_t, true>;

  std::tuple<std::size_t, int> add_row_to_filters() {
    return filters.add_row();
  }
  typename cross::thread_policy::mutex_type & lock() {
    return mutex;
  }

};
} //namespace impl
} //namespace cross
#include "impl/crossfilter_impl.ipp"

#endif
