/*This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2018 Dmitry Vinokurov */

#ifndef FILTERUTILS_H_GUARD
#define FILTERUTILS_H_GUARD
#include <type_traits>
#include <iterator>


namespace cross {

enum event {
  dataAdded,
  dataRemoved,
  dataFiltered
};
struct iterable {};
struct non_iterable {};

template<typename T>  struct trivial_hash {
  constexpr std::size_t operator()(const T &) const {
    return std::size_t();
  }
};

namespace impl {

template<typename T>
struct extract_container_value {
  using type = typename std::decay<decltype(*std::begin(std::declval<T>()))>::type;
};

template<typename T>
using extract_container_value_t = typename extract_container_value<T>::type;

template<typename T, typename F>
struct extract_value {
  using type = typename std::decay<decltype(*std::begin(std::declval<F>()(
      std::declval<T>())))>::type;
};

namespace trait {
template <typename, bool> struct cond_type;
template<typename T> struct cond_type<T, true> { using type = extract_container_value_t<T>;};
template<typename T> struct cond_type<T, false> { using type = T;};
}

template<typename Value, bool isIterable>
struct val {
  struct value_type1 : std::enable_if<isIterable, typename extract_container_value_t<Value>::type> {};
  struct value_type2 : std::enable_if<!isIterable, Value> {};
};

static constexpr int REMOVED_INDEX=-1;

}
}


// namespace detail {
// struct getter_type {};
// }

#endif
