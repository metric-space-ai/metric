/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include <numeric>
//#include "detail/dimension.hpp"
namespace cross {

template<typename V, typename T, typename I, typename H>
inline
dimension<V, T, I, H>::dimension(impl::filter_impl<T,H> *cf, std::size_t offset, int bit_num,
                                   std::function<field_type_t(const record_type_t &)> getter_)
    : impl::dimension_impl<V, T, I, H>(cf, offset, bit_num, getter_) {
}


template<typename V, typename T, typename I, typename H>
inline
void  dimension<V, T, I, H>::dispose() {
  impl::dimension_impl<V, T, I, H>::dispose();
}


template<typename V, typename T, typename I, typename H>
inline
void  dimension<V, T, I, H>::filter_range(const value_type_t & left, const value_type_t & right) {
  {
    writer_lock_t lk(crossfilter->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    impl::dimension_impl<V, T, I, H>::filter_range(left, right);
  }
  crossfilter->trigger_on_change(cross::dataFiltered);
}

template<typename V, typename T, typename I, typename H>
inline
void  dimension<V, T, I, H>::filter_exact(const value_type_t & value) {
  {
    writer_lock_t lk(crossfilter->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    impl::dimension_impl<V, T, I, H>::filter_exact(value);
  }
  crossfilter->trigger_on_change(cross::dataFiltered);
}

template<typename V, typename T, typename I, typename H>
inline
void  dimension<V, T, I, H>::filter_all() {
  {
    writer_lock_t lk(crossfilter->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    impl::dimension_impl<V, T, I, H>::filter_all();
  }
  crossfilter->trigger_on_change(cross::dataFiltered);
}

template<typename V, typename T, typename I, typename H>
inline
void
dimension<V, T, I, H>::filter_with_predicate(std::function<bool(const value_type_t&)> filter_function) {
  {
    writer_lock_t lk(crossfilter->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    impl::dimension_impl<V, T, I, H>::filter_with_predicate(filter_function);
  }
  crossfilter->trigger_on_change(cross::dataFiltered);
}
template<typename V, typename T, typename I, typename H>
inline
void  dimension<V, T, I, H>::filter_function(std::function<bool(const value_type_t&)> predicate) {
  {
    writer_lock_t lk(crossfilter->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    impl::dimension_impl<V, T, I, H>::filter_with_predicate(predicate);
  }
  crossfilter->trigger_on_change(cross::dataFiltered);
}

template<typename V, typename T, typename I, typename H>
inline
auto
dimension<V, T, I, H>::bottom(int64_t k, int64_t bottom_offset) const noexcept -> std::vector<record_type_t> {
  reader_lock_t lk(crossfilter->lock());
  (void)lk; // avoid AppleClang warning about unused variable;
  return impl::dimension_impl<V, T, I, H>::bottom(k, bottom_offset);
}

template<typename V, typename T, typename I, typename H>
inline
std::vector<typename dimension<V, T, I, H>::record_type_t>
dimension<V, T, I, H>::top(int64_t k, int64_t top_offset) const noexcept {
  reader_lock_t lk(crossfilter->lock());
  (void)lk; // avoid AppleClang warning about unused variable;
  return impl::dimension_impl<V, T, I, H>::top(k, top_offset);
}

template<typename V, typename T, typename I, typename H>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc, typename KeyFunc>
inline
auto dimension<V, T, I, H>::feature(
    AddFunc add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_,
    KeyFunc key) noexcept -> cross::feature<decltype(key(std::declval<value_type_t>())),
                          decltype(initial_func_()), this_type_t, false> {
  reader_lock_t lk(crossfilter->lock());
  (void)lk; // avoid AppleClang warning about unused variable;
  using K = decltype(key(std::declval<value_type_t>()));
  using R = decltype(initial_func_());
  return impl::dimension_impl<V, T, I, H>::template feature<K, R>(this,key, add_func_, remove_func_, initial_func_);
}

template<typename V, typename T, typename I, typename H>
template<typename F>
inline
auto
dimension<V, T, I, H>::feature_count(F key) noexcept ->
    cross::feature<decltype(key(std::declval<value_type_t>())),
          std::size_t, this_type_t, false> {
  return feature(
      [](std::size_t & r, const record_type_t &, bool ) {
        r++;
        return r;
      },
      [](std::size_t & r, const record_type_t &, bool ) {
        r--;
        return r;
      },
      []() {
        return std::size_t(0);
      },
      key);
}

template<typename V, typename T, typename I, typename H>
template<typename ValueFunc, typename KeyFunc>
inline
auto dimension<V, T, I, H>::feature_sum(ValueFunc value,
                                        KeyFunc key) noexcept 
    -> cross::feature<decltype(key(std::declval<value_type_t>())),
                      decltype(value(std::declval<record_type_t>())), this_type_t, false> {
  return feature(
      [value](auto & r, const auto & rec, bool) {
        r += value(rec);
        return r;// + value(rec);
      },
      [value](auto & r, const auto & rec, bool) {
        r -=value(rec);
        return r;// - value(rec);
      },
      []() {
        return decltype(value(std::declval<record_type_t>()))();
      },
      key);
}

template<typename V, typename T, typename I, typename H>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
inline
auto
dimension<V, T, I, H>::feature_all(
    AddFunc add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_)  noexcept -> cross::feature<std::size_t, decltype(initial_func_()), this_type_t, true> {
  reader_lock_t lk(crossfilter->lock());
  (void)lk; // avoid AppleClang warning about unused variable;
  return impl::dimension_impl<V, T, I, H>::template feature_all(this, add_func_, remove_func_, initial_func_);
}


template<typename V, typename T, typename I, typename H>
inline
auto
dimension<V, T, I, H>::feature_all_count() noexcept -> cross::feature<std::size_t, std::size_t, this_type_t, true> {
    return feature_all(
        [](std::size_t & r, const record_type_t &, bool ) {
          return r + 1;
        },
        [](std::size_t & r, const record_type_t &, bool ) {
          return r - 1;
        },
        []() {
          return std::size_t(0);
        });
  }

template<typename V, typename T, typename I, typename H>
template<typename G>
inline
auto dimension<V, T, I, H>::feature_all_sum(G value) noexcept 
    -> cross::feature<std::size_t, decltype(value(record_type_t())), this_type_t, true> {
  using reduce_type = decltype(value(record_type_t()));
  return feature_all(
      [value](reduce_type & r, const record_type_t & rec, bool) {
        return r + value(rec);
      },
      [value](reduce_type & r, const record_type_t & rec, bool) {
        return r - value(rec);
      },
      []() {
        return reduce_type();;
      });
}

} //namespace cross
