/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef DIMENSION_H_GUARD
#define DIMENSION_H_GUARD
#include <vector>
#include <functional>
#include <utility>
#include <tuple>
#include <type_traits>
#include "../detail/dimension_impl.hpp"
#include "../detail/utils.hpp"
#include "../detail/feature_impl.hpp"
#include "../detail/thread_policy.hpp"
#include "../detail/crossfilter.hpp"

namespace cross {
template <typename, typename, typename, bool> struct feature;

template <typename V, typename T, typename I = cross::non_iterable, typename H = cross::trivial_hash<T> >
struct  dimension : private impl::dimension_impl<V, T, I, H> {
  static constexpr bool isIterable = std::is_same<I,cross::iterable>::value;
  using value_type_t = typename impl::dimension_impl<V, T, I, H>::value_type_t;
  using field_type_t = typename impl::dimension_impl<V, T, I, H>::field_type_t;
  using record_type_t = typename impl::dimension_impl<V, T, I, H>::record_type_t;
  using this_type_t = dimension<V, T, I, H>;
  using impl_type_t = impl::dimension_impl<V, T, I, H>;
  using data_iterator = typename impl::dimension_impl<V,T,I,H>::data_iterator;
  using connection_type_t = typename impl::dimension_impl<V,T,I,H>::connection_type_t;
  template<typename F> using signal_type_t = typename impl::dimension_impl<V,T,I,H>::template signal_type_t<F>;
  using reader_lock_t = cross::thread_policy::read_lock_t;
  using writer_lock_t = cross::thread_policy::write_lock_t;
  using impl::dimension_impl<V,T,I,H>::crossfilter;

  template <typename A, typename B> friend struct impl::filter_impl;
  template <typename, typename, typename, bool> friend struct cross::impl::feature_impl;
  
  static constexpr  bool get_is_iterable() {
    return isIterable;
  }

  void add(std::size_t index, data_iterator begin, data_iterator end) {
    impl::dimension_impl<V,T,I,H>::add(index, begin, end);
  }
 public:

  //  static const std::function<value_type_t(const value_type_t &)> identity_function =

  dimension() {}

  dimension(cross::impl::filter_impl<T,H> *cf, std::size_t offset, int bit_num,
            std::function<field_type_t(const record_type_t &)> getter_);

  dimension(dimension<V, T, I, H> && dim)
      :impl::dimension_impl<V, T, I, H>(std::move(dim)) {  }

  dimension & operator = (dimension && dim) {
    if (this == &dim)
      return *this;
    impl::dimension_impl<V, T, I, H>::operator=(std::move(dim));
    return *this;
  }

  void dispose();

  std::size_t get_offset() const {
    return impl::dimension_impl<V, T, I, H>::dimension_offset;
  }

  std::size_t get_bit_index() const {
    return impl::dimension_impl<V, T, I, H>::dimension_bit_index;
  }
  /**
     Filters records such that this dimension's value is greater than or equal to left, and less than right.
   */
  void filter_range(const value_type_t &left, const value_type_t &right);
  /**
     Filters records such that this dimension's value equals value
  */
  void filter_exact(const value_type_t &value);

  /**
     Clears any filters on this dimension
  */
  void filter_all();

  /**
     Filters records such that the specified function returns truthy when called with this dimension's value
   */
  void filter_with_predicate(std::function<bool(const value_type_t &)> filterFunction);

  /**
     Filters records such that the specified function returns truthy when called with this dimension's value
   */
  void filter_function(std::function<bool(const value_type_t &)> predicate);

  /**
     Clears any filters on this dimension
   */
  void filter() { filter_all();}

  /**
     Filters records such that this dimension's value is greater than or equal to left, and less than right.
  */
  void filter(const value_type_t  & left, const value_type_t & right) { filter_range(left, right); }

  /**
     Filters records such that this dimension's value equals value
   */
  void filter(const value_type_t & value) { filter_exact(value); }

  /**
     Filters records such that the specified function returns truthy when called with this dimension's value
   */
  void filter(std::function<bool(const value_type_t &)> filterFunction) { filter_with_predicate(filterFunction); }

  /**
     Returns a new array containing the bottom k records, according to the natural order of this dimension.
     The returned array is sorted by ascending natural order. This method intersects the crossfilter's current filters,
     returning only records that satisfy every active filter (including this dimension's filter).
     Optionally, retrieve k records offset by offset
   */
  std::vector<record_type_t> bottom(int64_t k,
                                    int64_t  bottom_offset = 0) const noexcept ;

  /**
     Returns a new array containing the top k records, according to the natural order of this dimension.
     The returned array is sorted by descending natural order.
     This method intersects the crossfilter's current filters, returning only records that satisfy every
     active filter (including this dimension's filter). Optionally, retrieve k records offset by offset (records offset - offset + k - 1)
   */
  std::vector<record_type_t> top(int64_t k, int64_t top_offset = 0) const noexcept;

  /**
     Constructs a new grouping for the given dimension, according to the specified reduce functions.
   */
  template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
  auto
  feature(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_ ) noexcept
      -> cross::feature<value_type_t,
                        decltype(initial_func_()), this_type_t, false>  {
    return feature(add_func_, remove_func_, initial_func_, [](auto v) { return v;});
  }

  /**
     Constructs a new grouping for the given dimension, according to the specified reduce and key functions.
   */
  template <typename AddFunc, typename RemoveFunc, typename InitialFunc, typename KeyFunc>
  auto
  feature(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_,
      KeyFunc key) noexcept -> cross::feature<decltype(key(std::declval<value_type_t>())),
                                     decltype(initial_func_()), this_type_t, false>;


  auto  feature_count() noexcept -> cross::feature<value_type_t, std::size_t, this_type_t, false>{
    return feature_count([](auto v) { return v;});
  }

  /**
     Constructs a new grouping for the given dimension to reduce elements by count 
   */
  template<typename G>
  auto  feature(G key_) noexcept -> cross::feature<decltype(key_(std::declval<value_type_t>())),
                                          std::size_t, this_type_t, false> {
    return feature_count(key_);
  }

  /**
     Constructs a new grouping for the given dimension to reduce elements by count 
   */
  template<typename K>
  auto feature_count(K key) noexcept -> cross::feature<decltype(key(std::declval<value_type_t>())),
                                              std::size_t, this_type_t, false>;
  /**
     Constructs a new grouping for the given dimension to reduce elements by sum
   */
  template<typename ValueFunc>
  auto feature_sum(ValueFunc value) noexcept
      -> cross::feature<value_type_t, decltype(value(record_type_t())), this_type_t, false> {
    return feature_sum(value, [](auto v) { return v;});
  }

  /**
     Constructs a new grouping for the given dimension to reduce elements by sum
   */
  template<typename ValueFunc, typename KeyFunc>
  auto feature_sum(ValueFunc value,
                   KeyFunc key) noexcept -> cross::feature<decltype(key(std::declval<value_type_t>())),
                                                           decltype(value(std::declval<record_type_t>())), this_type_t, false>;
  /**
     A convenience function for grouping all records into a single group.  
   */
  template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
  auto
  feature_all(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_) noexcept -> cross::feature<std::size_t, decltype(initial_func_()), this_type_t, true>;

  /**
     A convenience function for grouping all records into a single group to reduce by count
   */
  auto feature_all_count() noexcept -> cross::feature<std::size_t, std::size_t, this_type_t, true>;

  /**
     A convenience function for grouping all records into a single group to reduce by sum
   */
  template<typename G>
  auto feature_all_sum(G value) noexcept -> cross::feature<std::size_t, decltype(value(record_type_t())), this_type_t, true>;

};
} //namespace cross
#include "../detail/impl/dimension.ipp"


#endif
