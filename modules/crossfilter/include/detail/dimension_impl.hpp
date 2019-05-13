/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef DIMENSION_IMPL_H_GUARD
#define DIMENSION_IMPL_H_GUARD
#include <vector>
#include <functional>
#include <utility>
#include <tuple>
#include <type_traits>
#include "../detail/crossfilter_impl.hpp"
#include "../detail/utils.hpp"
#include "../detail/thread_policy.hpp"

namespace cross {
template <typename, typename, typename, typename> struct dimension;
template <typename, typename, typename, bool> struct feature;

namespace impl {


template<typename, typename> struct filter_impl;

template <typename V, typename T, typename I,  typename H> struct dimension_impl  {
  static constexpr bool isIterable = std::is_same<I,cross::iterable>::value;
  using value_type_t = typename trait::cond_type<V, isIterable>::type;
  using field_type_t = V;
  using record_type_t = T;
  using this_type_t = cross::dimension<V, T, I, H>;
  using data_iterator = typename std::vector<T>::const_iterator;

  template<typename F> using signal_type_t = MovableSignal<typename signals::signal<F>, signals::connection>;
  using connection_type_t = signals::connection;

  template <typename, typename, typename,  bool>  friend struct feature;
  template <typename, typename, typename,  bool>  friend struct feature_impl;
  template <typename, typename> friend struct filter_impl;

  std::vector<value_type_t> values;
  std::vector<std::size_t> indexes;
  std::vector<value_type_t> new_values;
  std::vector<std::size_t> new_indexes;
  std::vector<std::size_t> added;
  std::vector<std::size_t> removed;
  std::size_t old_data_size = 0;
  std::size_t dimension_offset;
  int dimension_bit_index ;

  cross::impl::filter_impl<T,H> * crossfilter = nullptr;

  std::size_t low = 0;
  std::size_t high = 0;

  // iterable stuff
  std::vector<std::size_t> iterables_index_count;
  std::vector<std::size_t> iterables_empty_rows;


  std::function<std::tuple<std::size_t, std::size_t>(
      const std::vector<value_type_t> &values)>
      refilter;

  std::function<field_type_t(const record_type_t &)> getter;

  std::function<bool(value_type_t)> refilter_function;

  bool refilter_function_flag = false;

  std::function<void(std::size_t)> slot_post_add;
  connection_type_t connection_post_add;

  std::function<void()> slot_clear;
  connection_type_t connection_clear;

  std::function<void(std::size_t, data_iterator, data_iterator)> slot_add;
  connection_type_t connection_add;

  std::function<void(const std::vector<int64_t> &)> slot_remove;
  connection_type_t connection_remove;

  signal_type_t<void(const std::vector<int64_t> &)> remove_signal;
  signal_type_t<void()> dispose_dimension_signal;
  signal_type_t<void(const std::vector<value_type_t> &,
                               const std::vector<std::size_t> &, std::size_t, std::size_t)>  add_signal;

  template<bool Enable = true>
  std::enable_if_t<isIterable && Enable> add(std::size_t insert_point, data_iterator begin, data_iterator end);

  template<bool Enable = true>
  std::enable_if_t<!isIterable && Enable> add(std::size_t insert_point, data_iterator begin, data_iterator end);

  bool zero_except(std::size_t index) ;

  bool
  zero_except(std::size_t index, std::size_t offset, int bit_index) {
    return crossfilter->zero_except(index, offset, bit_index);
  }

  bool only_except(std::size_t index, std::size_t offset, int bitIndex) ;


  connection_type_t connect_filter_slot(
      std::function<void(std::size_t filter_offset, int filter_bit_num,
                         const std::vector<std::size_t> &,
                         const std::vector<std::size_t> &, bool)> slot) ;

  connection_type_t
  connect_add_slot(std::function<void(const std::vector<value_type_t> &,
                                    const std::vector<std::size_t> &, std::size_t, std::size_t)> slot) ;

  connection_type_t
  connect_remove_slot(std::function<void(const std::vector<int64_t> &)> slot) ;

  connection_type_t
  connect_dispose_slot(std::function<void()> slot) ;

  connection_type_t
  connect_clear_slot(std::function<void()> slot)  {
    return crossfilter->connect_clear_slot(slot);
  }

  void remove_data(const std::vector<int64_t> &re_index);

  void remove_data_iterable(const std::vector<int64_t> &re_index);

  std::tuple<std::size_t, std::size_t>
  refilter_range(const value_type_t & left, const value_type_t &right,
                const std::vector<value_type_t> &data);

  std::tuple<std::size_t, std::size_t>
  refilter_exact(const value_type_t &value, const std::vector<value_type_t> &data);

  template<bool Enable = true>
  std::enable_if_t<isIterable && Enable>
  do_filter(std::size_t filter_low_index, std::size_t filter_high_index);

  template<bool Enable = true>
  std::enable_if_t<!isIterable && Enable>
  do_filter(std::size_t filter_low_index, std::size_t filter_high_index);

 public:
  ~dimension_impl() {
    //spdlog::get("console")->info("~dimension_impl() = {:x} , {:x}", (uint64_t)add_signal.signal.get(), (uint64_t)this);
  }

  dimension_impl() {
    //spdlog::get("console")->info("dimension_impl()  {:x}", (uint64_t)this);
  }

  dimension_impl(cross::impl::filter_impl<T,H> *cf, std::size_t offset, int bit_num,
            std::function<field_type_t(const record_type_t &)> getter_);

  dimension_impl(dimension_impl<V, T, I, H> && dim)
      : values(dim.values), indexes(dim.indexes),
        dimension_offset(dim.dimension_offset), dimension_bit_index(dim.dimension_bit_index),
        crossfilter(dim.crossfilter),
        low(dim.low), high(dim.high), iterables_index_count(dim.iterables_index_count),
        iterables_empty_rows(dim.iterables_empty_rows), refilter(dim.refilter),
        getter(dim.getter), refilter_function_flag(dim.refilter_function_flag),
        remove_signal(std::move(dim.remove_signal)),
        dispose_dimension_signal(std::move(dim.dispose_dimension_signal)),
        add_signal(std::move(dim.add_signal))
  {
    slot_add =  [this](std::size_t index, data_iterator begin, data_iterator end) {
                  this->add(index, begin, end); };

    slot_remove = [this] (const std::vector<int64_t> &re_index) {
      this->remove_data(re_index);
    };

    slot_post_add = [this](std::size_t new_data_size) {
      add_signal(new_values, new_indexes, old_data_size, new_data_size);
      new_values.clear();
    };
    slot_clear = [this]() { clear(); };
    dim.connection_clear.disconnect();
    connection_clear = crossfilter->connect_clear_slot(slot_clear);
    dim.connection_add.disconnect();
    connection_add = crossfilter->connect_add_slot(slot_add);
    dim.connection_remove.disconnect();
    connection_remove = crossfilter->connect_remove_slot(slot_remove);
    dim.connection_post_add.disconnect();
    connection_post_add = crossfilter->connect_post_add_slot(slot_post_add);
    //spdlog::get("console")->info("dimension(dim &&) = {:x}, {:x}, {:x}" , (uint64_t)add_signal.signal.get(),(uint64_t)this, (uint64_t)(&dim));
  }

  dimension_impl & operator = (dimension_impl<V, T, I, H> && dim) {
    //    spdlog::get("console")->info("operator_impl=(&&)  {:x},{:x}", (uint64_t)this, (uint64_t)(&dim));
    values = std::move(dim.values);
    indexes = std::move(dim.indexes);
    crossfilter = std::move(dim.crossfilter);
    dimension_offset = dim.dimension_offset;
    dimension_bit_index = dim.dimension_bit_index;
    low = dim.low;
    high = dim.high;
    iterables_index_count = std::move(dim.iterables_index_count);
    iterables_empty_rows = std::move(dim.iterables_empty_rows);
    refilter = std::move(dim.refilter);
    getter = std::move(dim.getter);
    refilter_function_flag = dim.refilter_function_flag;
    remove_signal = std::move(dim.remove_signal);
    dispose_dimension_signal = std::move(dim.dispose_dimension_signal);
    //    add_signal = std::move(dim.add_signal);
    // slot_add =  [this](std::size_t index, data_iterator begin, data_iterator end) {
    //               this->add(index, begin, end); };
    slot_remove = [this] (const std::vector<int64_t> &reindex) {
      this->remove_data(reindex);
    };
    slot_post_add = [this](std::size_t new_data_size) {
      add_signal(new_values, new_indexes, old_data_size, new_data_size);
      new_values.clear();
    };
    slot_clear = [this]() { clear(); };
    dim.connection_clear.disconnect();
    connection_clear = crossfilter->connect_clear_slot(slot_clear);

    connection_add.disconnect();
    dim.connection_add.disconnect();
    connection_add = crossfilter->connect_add_slot([this](std::size_t index, data_iterator begin, data_iterator end) {
                                                     this->add(index, begin, end); });

    dim.connection_remove.disconnect();
    connection_remove.disconnect();
    connection_remove = crossfilter->connect_remove_slot(slot_remove);
    dim.connection_post_add.disconnect();
    connection_post_add.disconnect();
    connection_post_add = crossfilter->connect_post_add_slot(
        [this](std::size_t new_data_size) {
          //    spdlog::get("console")->info("dim() pos_add = {:x}",(uint64_t)add_signal.signal.get());
          add_signal(new_values, new_indexes, old_data_size, new_data_size);
          new_values.clear();
        });
    //    spdlog::get("console")->info("operator = (dim &&) = {:x}" , (uint64_t)add_signal.signal.get());
    return *this;
  }

  void clear();

  void dispose();

  void filter_range(const value_type_t &left, const value_type_t &right);

  void filter_exact(const value_type_t &value);

  void filter_all();

  void filter_with_predicate(std::function<bool(const value_type_t &)> filter_function);

  std::vector<record_type_t> bottom(int64_t k,
                                    int64_t bottom_offset = 0) const noexcept ;

  std::vector<record_type_t> top(int64_t k, int64_t top_offset = 0) const noexcept ;

  template <typename K, typename R>
  auto
  feature(cross::dimension<V,T,I,H> * base,
          std::function<K(const value_type_t &)> key,
          std::function<R(R &, const record_type_t &, bool)> add_func_,
          std::function<R(R &, const record_type_t &, bool)> remove_func_,
          std::function<R()> initial_func_) noexcept -> cross::feature<K, R, this_type_t, false>;

  template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
  auto
  feature_all(
      cross::dimension<V,T,I,H> * base,
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_) noexcept  ->  cross::feature<std::size_t, decltype(initial_func_()), this_type_t, true>;

  std::size_t translate_index(std::size_t index) const noexcept{
    return indexes[index];
  }
  const T & get_raw(std::size_t index) const noexcept {
    return crossfilter->get_raw(index);
  }

  void filter1(std::vector<std::size_t> & added, std::vector<std::size_t>& removed, std::size_t filterLowIndex, std::size_t filterHighIndex);
  void filter2(std::vector<std::size_t> & added, std::vector<std::size_t>& removed, std::size_t filterLowIndex, std::size_t filterHighIndex);

  cross::thread_policy::mutex_type & lock() {
    return crossfilter->lock();
  }

};
} //namespace impl
} //namespace cross
#include "impl/dimension_impl.ipp"


#endif
