/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef CROSSFILTER_DETAIL_H_GUARD
#define CROSSFILTER_DETAIL_H_GUARD
#include <functional>
#include <vector>
#include <type_traits>
#include <sstream>
#include <cstddef>

#include "../detail/crossfilter_impl.hpp"
#include "../detail/dimension_impl.hpp"
#include "../detail/feature_impl.hpp"

#include "../detail/utils.hpp"
#include "../detail/thread_policy.hpp"

namespace cross {

template <typename, typename, typename, typename> struct dimension;

template <typename T, typename Hash = trivial_hash<T>> struct filter: private impl::filter_impl<T,Hash> {
 public:
  
  using this_type_t = filter<T,Hash>;
  using impl_type_t = impl::filter_impl<T,Hash>;
  using record_type_t = typename impl::filter_impl<T,Hash>::record_type_t;
  using value_type_t = typename impl::filter_impl<T,Hash>::value_type_t;
  template<typename U> using dimension_t = dimension<U, T, cross::non_iterable, Hash>;
  template<typename U> using iterable_dimension_t = dimension<U, T, cross::iterable, Hash>;
  using data_iterator = typename impl::filter_impl<T,Hash>::data_iterator;
  using base_type_t = impl::filter_impl<T,Hash>;
  using connection_type_t = typename impl::filter_impl<T,Hash>::connection_type_t;
  template<typename F> using signal_type_t = typename impl::filter_impl<T,Hash>::template signal_type_t<F>;


  template <typename, typename, typename, bool> friend struct impl::feature_impl;
  template <typename, typename, typename, typename> friend struct impl::dimension_impl;
  using reader_lock_t = cross::thread_policy::read_lock_t;
  using writer_lock_t = cross::thread_policy::write_lock_t;
  using impl::filter_impl<T,Hash>::mutex;
  signal_type_t<void(cross::event)>           on_change_signal;
 private:
  bool empty_unlocked() const {
    return impl_type_t::data.empty();
  }
  void trigger_on_change(cross::event event)  override {
    on_change_signal(event);
  }
 public:
  struct iterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;
   private:
    std::size_t index;
    std::vector<T> & data;
    iterator(std::size_t i, std::vector<T> & d):index(i),data(d) {}
    friend struct filter;

   public:

    iterator(const iterator & i):index(i.index),data(i.data) {}
    iterator & operator = (iterator && i) {
      if(&i == this)
        return *this;
      index = i.index;
      data = i.data;
      return *this;
    }
    bool operator == (const iterator & i) const noexcept {
      return index == i.index;
    }
    bool operator != (const iterator & i) const noexcept{
      return !(*this == i);
    }
    bool operator < (const iterator & i) const noexcept{
      return index < i.index;
    }
    bool operator > (const iterator & i) const noexcept{
      return index > i.index;
    }

    iterator & operator++() {
      index++;
      return *this;
    }
    iterator operator++(int) {
      auto result(*this);
      index++;
      return result;
    }
    iterator & operator--() {
      index--;
      return *this;
    }
    iterator operator--(int) {
      auto result(*this);
      index--;
      return result;
    }

    const T & operator *() const {
      return data[index];
    }
    T const * operator ->() {
      return &data[index];
    }
  };
  struct reverse_iterator {
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

   private:
    iterator current;
    friend struct filter;
   public:
    reverse_iterator(const iterator & x): current(x) {}
    reverse_iterator(const reverse_iterator & i):current(i.current) {}
    reverse_iterator & operator = (reverse_iterator && i) {
      if(&i == this)
        return *this;
      current = std::move(i.current);
      return *this;
    }
    bool operator == (const reverse_iterator & i) const noexcept {
      return current == i.current;
    }
    bool operator != (const reverse_iterator & i) const noexcept{
      return !(*this == i);
    }
    bool operator < (const reverse_iterator & i) const noexcept{
      return current < i.current;
    }
    bool operator > (const reverse_iterator & i) const noexcept{
      return current > i.current;
    }

    reverse_iterator & operator++() {
      current--;
      return *this;
    }
    reverse_iterator operator++(int) {
      auto result(*this);
      current--;
      return result;
    }
    reverse_iterator & operator--() {
      current++;
      return *this;
    }
    reverse_iterator operator--(int) {
      auto result(*this);
      current++;
      return result;
    }
    const T & operator *() const {
      iterator tmp = current;
      return *--tmp;
    }
    T const * operator ->() {
      return &(operator*());
    }
  };

  static constexpr bool get_is_iterable() {
    return false;
  }

  /**
     Create crossfilter with empty data
   */
  filter(Hash h = Hash()):impl_type_t(h) {}

  /**
     Create crossfilter and add data from 'data'
     \param[in]  data Container of elements with type T, must support std::begin/end concept
   */
  template<typename C>
  explicit filter(const C & data, Hash h = Hash())
      :impl_type_t(std::begin(data), std::end(data), h) {}

  /**
     Constructs the container with the contents of the initializer list 
     \param[in] init initializer list
  */

  filter(std::initializer_list<T>  init, Hash h = Hash()):impl_type_t(init.begin(),init.end(), h) {  }

  #ifdef CROSS_FILTER_USE_THREAD_POOL
  /**
     set size of internal thread pool
     \param[in] thread_num - amount of threads in thread pool
  */
  void set_thread_pool_size(uint32_t thread_num ) {
    impl_type_t::set_thread_pool_size(thread_num);
  }
  #endif
  /**
     Add all data from container to crossfilter
     \param[in] new_data Container of elements with type T, must support std::begin/end concept
   */
  template<typename C>
  typename std::enable_if<!std::is_same<C, T>::value, filter&>::type
  add(const C &new_data, bool allow_duplicates = true);

  /**
     Add one element of type T to crossfilter
     \param[in] new_data - element to be added to crossfilter
   */
  filter &add(const T &new_data, bool allow_duplicates = true);

  /**
     Returns the number of records in the crossfilter, independent of any filters
   */
  std::size_t size() const;

  /**
     Returns all of the raw records in the crossfilter, independent of any filters
   */
  std::vector<T> all() const {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    return impl_type_t::all();
  }

  /**
     Returns all of the raw records in the crossfilter, with filters applied.
     Can optionally ignore filters that are defined on  dimension list [dimensions]
   */
  template<typename ...Ts>
  std::vector<T> all_filtered(Ts&... dimensions) const {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    return impl_type_t::all_filtered(dimensions...);
  }
  /**
     auxilary method for n-api. analog of all_filtered
   */
  std::vector<T> all_filtered_except_mask(const std::vector<uint8_t> & mask) const {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    return impl_type_t::all_filtered_except_mask(mask);
  }

  /**
     Check if the data record at index i is excluded by the current filter set.
     Can optionally ignore filters that are defined on  dimension list [dimensions]
   */
  template<typename ...Ts>
  bool is_element_filtered(std::size_t index, Ts&... dimensions) const {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    return impl_type_t::is_element_filtered(index,dimensions...);
  }
  bool is_element_filtered_except_mask(std::size_t index,const std::vector<uint8_t> & mask) const {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    return impl_type_t::is_element_filtered_except_mask(index,mask);
  }

  /**
     Constructs a new dimension using the specified value accessor function.
   */
  template<typename F>
  auto dimension(F getter) -> cross::dimension<decltype(getter(std::declval<record_type_t>())), T, cross::non_iterable, Hash> {
    //    writer_lock_t lk(mutex);
    return impl_type_t::dimension(getter);
  }

  /**
     Constructs a new dimension using the specified value accessor function.
     Getter function must return container of elements.
  */
  template<typename F>
  auto iterable_dimension(F getter) ->cross::dimension<decltype(getter(std::declval<record_type_t>())), T, cross::iterable, Hash> {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    using value_type = decltype(getter(record_type_t()));
    return impl_type_t::template iterable_dimension<value_type>(getter);
  }


  /**
     removes all records matching the predicate (ignoring filters).
  */
  void remove(std::function<bool(const T&, int)> predicate);

  /**
     Removes all records that match the current filters
  */
  void remove();

  /**
     A convenience function for grouping all records and reducing to a single value,
     with given reduce functions.
   */
  template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
  auto
  feature(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_) -> feature<std::size_t,
                                            decltype(initial_func_()), this_type_t, true>;

  /**
     A convenience function for grouping all records and reducing to a single value,
     with predefined reduce functions to count records
  */

  auto feature_count() ->  cross::feature<std::size_t, std::size_t, this_type_t, true>;

  /**
     A convenience function for grouping all records and reducing to a single value,
     with predefined reduce functions to sum records
  */
  template<typename G>
  auto feature_sum(G value) -> cross::feature<std::size_t, decltype(value(record_type_t())), this_type_t, true>;

  // /**
  //    Equivalent ot groupAllReduceCount()
  // */
  // cross::feature<std::size_t, std::size_t, this_type_t, true> feature();

  /**
     Calls callback when certain changes happen on the Crossfilter. Crossfilter will pass the event type to callback as one of the following strings:
     * dataAdded
     * dataRemoved
     * filtered
   */
  connection_type_t onChange(std::function<void(cross::event)> callback) {
    return on_change_signal.connect(callback);
  }

  void assign(std::size_t n, const T & val, bool allow_duplicate = true) {
    {
      writer_lock_t lk(mutex);
      (void)lk; // avoid AppleClang warning about unused variable;
      if(!this->empty_unlocked()) {
        impl_type_t::clear();
      }
      if(!allow_duplicate) {
        impl_type_t::add(impl_type_t::size(),val, allow_duplicate);
      } else {
        for(std::size_t i = 0; i < n; i++)
          impl_type_t::add(impl_type_t::size(),val, allow_duplicate);
      }
    }
    on_change_signal(cross::dataAdded);
  }

  template<typename InputIterator>
  void assign(InputIterator first, InputIterator last, bool allow_duplicate = true) {
    {
      writer_lock_t lk(mutex);
      (void)lk; // avoid AppleClang warning about unused variable;
      if(!empty_unlocked()) {
        impl_type_t::clear();
      }
      impl_type_t::add(0,first,last,allow_duplicate);
    }
    on_change_signal(cross::dataAdded);
  }

  void assign(std::initializer_list<T>  l, bool allow_duplicate = true) {
    assign(l.begin(),l.end(), allow_duplicate);
  }

  const T & at(std::size_t n) const {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    if(n > impl_type_t::size()-1) {
      std::ostringstream os;
      os << "n (which is " << n << ") >= " << size() << " (which is " << size() << ")";
      throw std::out_of_range(os.str());
    }
    return impl_type_t::get_raw(n);
  }
  const T & back() const noexcept {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    return impl_type_t::data.back();
  }
  iterator begin() noexcept {
    return  iterator(0,impl_type_t::data);
  }
  iterator end() noexcept {
    return  iterator(impl_type_t::size(),impl_type_t::data);
  }
  iterator cbegin() const noexcept {
    return  iterator(0,impl_type_t::data);
  }
  iterator cend() const noexcept {
    return  iterator(size(),impl_type_t::data);
  }
  void clear() {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    impl_type_t::clear();

  }
  reverse_iterator crbegin() const noexcept {
    return reverse_iterator(end());
  }
  reverse_iterator crend() const noexcept {
    return reverse_iterator(begin());
  }
  reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }
  reverse_iterator rend()  noexcept {
    return reverse_iterator(begin());
  };
  bool empty() const noexcept {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    return size() == 0;
  }
  iterator erase(iterator position) {
    std::size_t dist = 0;
    mutex.lock();
    dist = std::distance(begin(),position);
    impl_type_t::remove(position.index, position.index+1);
    auto p = begin();
    std::advance(p,dist);
    mutex.unlock();
    on_change_signal(cross::dataRemoved);
    return p;
  }
  iterator erase( iterator first, iterator last) {
    mutex.lock();
    std::size_t dist = std::distance(begin(),last);
    impl_type_t::remove(first.index,last.index);
    auto p = begin();
    std::advance(p,dist);
    mutex.unlock();
    on_change_signal(cross::dataRemoved);
    return p;
  }
  const T& front() const noexcept {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    return impl_type_t::data.front();
  }
  iterator insert( iterator pos, const T & x, bool allow_duplicates = true) {
    {
      writer_lock_t lk(mutex);
      (void)lk; // avoid AppleClang warning about unused variable;
      impl_type_t::add(pos.index,x, allow_duplicates);
    }
    on_change_signal(cross::dataAdded);
    return iterator(pos.index,impl_type_t::data);
  }

  iterator insert( iterator pos, std::initializer_list<T> l, bool allow_duplicates = true) {
    {
      writer_lock_t lk(mutex);
      (void)lk; // avoid AppleClang warning about unused variable;
      impl_type_t::add(pos.index,l.begin(),l.end(), allow_duplicates);
    }
    on_change_signal(cross::dataAdded);
    return iterator(pos.index,impl_type_t::data);
  }
  iterator insert( iterator pos, std::size_t n, const T & x, bool allow_duplicates = true) {
    {
      writer_lock_t lk(mutex);
      (void)lk; // avoid AppleClang warning about unused variable;
      if(pos == end()) {
        std::vector<T> tmp(n,x);
        impl_type_t::add(tmp,allow_duplicates);
      } else {
        std::vector<T> tmp(impl_type_t::data.begin() + pos.index, impl_type_t::data.end());
        tmp.insert(tmp.begin(),n,x);
        impl_type_t::add(pos.index,tmp.begin(),tmp.end(), allow_duplicates);
      }
    }
    on_change_signal(cross::dataAdded);
    return iterator(pos.index,impl_type_t::data);
  }

  template<typename InputIterator>
  iterator insert( iterator pos, InputIterator first, InputIterator last, bool allow_duplicates = true) {
    {
      writer_lock_t lk(mutex);
      (void)lk; // avoid AppleClang warning about unused variable;
      impl_type_t::add(pos.index,first,last,allow_duplicates);
    }
    return iterator(pos.index,impl_type_t::data);
  }

  filter & operator= (const filter &x) {
    assign(x.begin(),x.end());
  }

  filter & operator= (filter &&x) noexcept {
    assign(x.begin(),x.end());
  }
  filter & operator= (std::initializer_list<T> l) {
    assign(l.begin(), l.end());
  }
  const T& operator[] (std::size_t n) const noexcept {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    return impl_type_t::get_raw(n);
  }
  void pop_back () noexcept {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    std::size_t last = size()-1;
    impl_type_t::remove([last](const auto&, int i ) { return std::size_t(i) == last;});
  }
  void push_back (const T &x, bool allow_duplicates = true) {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    impl_type_t::add(impl_type_t::data.size(),x, allow_duplicates);
  }
  // void push_back (T && x, bool allow_duplicates = true) {
  //   writer_lock_t lk(mutex);
  //   impl_type_t::add(std::move(x),allow_duplicates);
  // }
  const T * data () const noexcept {
    reader_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    return impl_type_t::data.data();
  }

  template<typename... _Args> iterator emplace (iterator position, _Args &&... args) {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    auto index = impl_type_t::emplace(position.index,args...);
    return iterator(index,data);
  }
  template<typename... _Args> void emplace_back (_Args &&... args) {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    impl_type_t::emplace(end().index,args...);
  }

  void reserve (std::size_t n) {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    impl_type_t::data.reserve(n);
  }
  void shrink_to_fit () {
    writer_lock_t lk(mutex);
    (void)lk; // avoid AppleClang warning about unused variable;
    impl_type_t::data.shrink_to_fit();
  }
  //void swap (filter & x) noexcept;
  std::size_t filters_size() const {
    return impl_type_t::filters.size();
  }  
};
}
#include "impl/crossfilter.ipp"


#endif
