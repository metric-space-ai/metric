/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef GROUP_INDEX_H_GUARD
#define GROUP_INDEX_H_GUARD
#include <vector>
#include <functional>
#include <type_traits>
#include <utility>
template<bool Flat = true>
struct GroupIndex {
  using index_type_t = typename std::conditional<Flat, std::size_t,std::vector<std::size_t>>::type;

  std::vector<index_type_t> data;

  GroupIndex() = default;

  explicit GroupIndex(std::size_t n)
      :data(n) {}

  GroupIndex(GroupIndex && gi) noexcept
      :data(std::move(gi.data)) {}

  GroupIndex & operator = (GroupIndex && gi) noexcept {
    data = std::move(gi.data);
    return *this;
  }
  std::size_t size() const {
    return data.size();
  }
  template<bool B = true>
  typename std::enable_if<B && Flat,std::size_t>::type
  size(std::size_t i) {
    (void)i;
    return 1;
  }
  template<bool B = true>
  typename std::enable_if<B && !Flat,std::size_t>::type
  size(std::size_t i) {
    return data[i].size();
  }
  void clear() {
    data.clear();
  }
  void resize(std::size_t newSize) {
    data.resize(newSize);
  }
  template<typename F, bool B = true>
  typename std::enable_if<B && Flat,void>::type
  for_each(std::size_t i, F functor) {
    functor(data[i]);
  }
  template<typename F, bool B = true>
  typename std::enable_if<B && !Flat,void>::type
  for_each(std::size_t i, F functor) {
    auto & e  = data[i];
    for(auto t : e) {
      functor(t);
    }
  }
  template<typename F, bool B = true>
  typename std::enable_if<B && Flat,void>::type
  set(std::size_t i, F functor) {
    data[i] = functor(data[i]);
  }
  template<typename F, bool B = true>
  typename std::enable_if<B && !Flat,void>::type
  set(std::size_t i, F functor) {
    auto & e  = data[i];
    for(auto &t : e) {
      t = functor(t);
    }
  }
  template<bool B = true>
  typename std::enable_if<B && Flat,void>::type
  setIndex(std::size_t index, std::size_t value) {
    data[index] = value;
  }
  template<bool B = true>
  typename std::enable_if<B && !Flat,void>::type
  setIndex(std::size_t index, std::size_t value) {
    data[index].push_back(value);
  }

  index_type_t & operator [] (std::size_t i) {
    return data[i];
  }
  const index_type_t & operator [] (std::size_t i) const {
    return data[i];
  }

};

#endif
