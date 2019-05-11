/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include "../crossfilter.hpp"
//#include "spdlog/spdlog.h"
namespace cross {

template<typename T, typename H>
template<typename C>
inline
typename std::enable_if<!std::is_same<C, T>::value, filter<T,H>&>::type
filter<T,H>::add(const C &new_data, bool allow_duplicates) {
  //  auto logger = spdlog::get("console");
  {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    //    if(logger) logger->info("dataAdded_pre");
    impl_type_t::add(new_data, allow_duplicates);
  }
  //  if(logger) logger->info("dataAdded_post");
  on_change_signal(cross::dataAdded);
  return *this;
}

template<typename T, typename H>
inline
filter<T,H> & filter<T,H>::add(const T &new_data, bool allow_duplicates) {
  //  auto logger = spdlog::get("console");
  {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    //    if(logger) logger->info("dataAdded_pre");
    impl_type_t::add(impl_type_t::size(),new_data, allow_duplicates);
  }
  //  if(logger) logger->info("dataAdded_post");
  on_change_signal(cross::dataAdded);
  return *this;
}

template<typename T, typename H>
inline
std::size_t filter<T,H>::size() const {
  reader_lock_t lk(mutex);
  (void)lk; // avoid AppleClang warning about unused variable;
  return impl_type_t::size();
}

// removes all records matching the predicate (ignoring filters).
template<typename T, typename H>
inline
void filter<T,H>::remove(std::function<bool(const T&, int)> predicate) {
  {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    impl_type_t::remove(predicate);
  }
  on_change_signal(cross::dataRemoved);
}

// Removes all records that match the current filters
template<typename T, typename H>
inline
void filter<T,H>::remove() {
  {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    impl_type_t::remove();
  }
  on_change_signal(cross::dataRemoved);
}

template<typename T, typename H>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
inline
auto
filter<T,H>::feature(
    AddFunc add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_) -> cross::feature<std::size_t,
                                        decltype(initial_func_()), this_type_t, true> {
  reader_lock_t lk(mutex);
  (void)lk; // avoid AppleClang warning about unused variable;
  return impl_type_t::feature(this, add_func_, remove_func_, initial_func_);
}

template <typename T, typename H>
inline
auto
filter<T,H>::feature_count() -> cross::feature<std::size_t, std::size_t, this_type_t, true> {
  return feature(
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

template<typename T, typename H>
template<typename G>
inline
auto filter<T,H>::feature_sum(G value)
    -> cross::feature<std::size_t, decltype(value(record_type_t())), this_type_t, true> {
  using R = decltype(value(record_type_t()));

  return feature(
        [value](R & r, const record_type_t & rec, bool) {
          return r + value(rec);
        },
        [value](R & r, const record_type_t & rec, bool) {
          return r - value(rec);
        },
        []() {
          return R();
        });
  }

// template<typename T, typename H>
// inline
// feature<std::size_t, std::size_t, typename filter<T,H>::this_type_t, true>
// filter<T,H>::feature() {
//   return feature_count();
// }

} // namespace cross
