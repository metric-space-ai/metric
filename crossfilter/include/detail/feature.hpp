/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef FEATURE_H_GUARD
#define FEATURE_H_GUARD
#include <vector>
#include <functional>
#include <unordered_map>
#include <utility>
#include <type_traits>
//#include <boost/signals2.hpp>
#include "../detail/feature_impl.hpp"
#include "../detail/crossfilter_impl.hpp"
#include "../detail/dimension_impl.hpp"
#include "../detail/thread_policy.hpp"

namespace cross {
template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll = false>
struct feature: private impl::feature_impl<Key, Reduce, Dimension, isGroupAll> {
  using feature_impl_t = impl::feature_impl<Key, Reduce,Dimension, isGroupAll>;

 public:
  using group_type_t = typename feature_impl_t::group_type_t;
  using reduce_type_t = typename feature_impl_t::reduce_type_t;
  using record_type_t = typename feature_impl_t::record_type_t;
  using value_type_t = typename feature_impl_t::value_type_t;

  using this_type_t = feature<Key, Reduce, Dimension, isGroupAll>;

  using reader_lock_t = cross::thread_policy::read_lock_t;
  using writer_lock_t = cross::thread_policy::write_lock_t;

 private:

  template <typename T,typename H> friend struct impl::filter_impl;
  template <typename ,typename , typename , typename> friend struct impl::dimension_impl;
  template<typename KeyFunc, typename AddFunc, typename RemoveFunc, typename InitialFunc, typename Base>
  feature(Base *dim, KeyFunc key_,
          AddFunc add_func_,
          RemoveFunc remove_func_,
          InitialFunc initial_func_)
      : feature_impl_t(dim, key_, add_func_, remove_func_, initial_func_) {}


 public:

  void dispose() { feature_impl_t::dispose();}

  feature(feature<Key, Reduce, Dimension, isGroupAll> && g):
      feature_impl_t(std::move(g)) {}


  /**
     Returns a new array containing the top k groups,
     according to the group order of the associated reduce value. The returned array is in descending order by reduce value.
   */
  std::vector<group_type_t> top(std::size_t k) noexcept {
    writer_lock_t lk(feature_impl_t::dimension->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    return feature_impl_t::top(k);
  }

  /**
     Returns a new array containing the top k groups,
     according to the order defined by orderFunc of the associated reduce value.
   */
  template<typename OrderFunc>
  std::vector<group_type_t> top(std::size_t k, OrderFunc orderFunc) noexcept {
    writer_lock_t lk(feature_impl_t::dimension->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    return feature_impl_t::top(k, orderFunc);
  }

  /**
     Returns the array of all groups, in ascending natural order by key. 
   */
  std::vector<group_type_t> &all() noexcept{
    writer_lock_t lk(feature_impl_t::dimension->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    return feature_impl_t::all();
  }

  /**
     Equivalent to all()[0].second.
   */
  reduce_type_t value() noexcept {
    writer_lock_t lk(feature_impl_t::dimension->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    return feature_impl_t::value();
  }

  /**
     Specifies the order value for computing the top-K groups.
     The default order is the identity function,
     which assumes that the reduction values are naturally-ordered (such as simple counts or sums).
   */
  template<typename OrderFunc>
  this_type_t &   order(OrderFunc value) noexcept {
    writer_lock_t lk(feature_impl_t::dimension->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    feature_impl_t::order(value);
    return *this;
  }

  /**
     A convenience method for using natural order for reduce values. Returns this grouping.
   */
  this_type_t & order_natural() noexcept{
    writer_lock_t lk(feature_impl_t::dimension->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    feature_impl_t::order_natural();
    return *this;
  }
  /**
     Returns the number of distinct values in the group, independent of any filters; the cardinality.
   */
  std::size_t size() const noexcept {
    reader_lock_t lk(feature_impl_t::dimension->lock());
    (void)lk; // avoid AppleClang warning about unused variable;
    return feature_impl_t::size();
  }

 
};
} //namespace cross
#endif
