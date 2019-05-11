/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include "../../detail/crossfilter_impl.hpp"
#include <numeric>
namespace {
static constexpr int REMOVED_INDEX = -1;
}
namespace cross {
namespace impl {

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t filter_impl<T,H>::connect_filter_slot(std::function<void(std::size_t, int,
                                                                                                      const std::vector<std::size_t> &,
                                                                                                      const std::vector<std::size_t> &, bool)> slot) {
  return filter_signal.connect(slot);
}

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_remove_slot(std::function<void(const std::vector<int64_t> &)> slot) {
  return remove_signal.connect(slot);
}

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_add_slot(std::function<void(const std::vector<value_type_t> &,
                                                      const std::vector<std::size_t> &,
                                                      std::size_t, std::size_t)> slot) {
  return add_group_signal.connect(slot);
}

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_dispose_slot(std::function<void()> slot)  {
  return  dispose_signal.connect(slot);
}

// template<typename T, typename H>
// inline
// typename filter_impl<T,H>::connection_type_t
// filter_impl<T,H>:: on_change(std::function<void(const std::string &)> callback) {
//   return on_change_signal.connect(callback);
// }

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_add_slot(std::function<void(std::size_t,data_iterator, data_iterator)> slot) {
  return add_signal.connect(slot);
}
template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_post_add_slot(std::function<void(std::size_t)> slot) {
  return post_add_signal.connect(slot);
}

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_clear_slot(std::function<void()> slot) {
  return clear_signal.connect(slot);
}

template<typename T, typename H>
template<typename G>
inline
auto filter_impl<T,H>::dimension(G getter)
    -> cross::dimension<decltype(getter(std::declval<T>())), T, cross::non_iterable,H> {
  auto dimension_filter = add_row_to_filters();
  cross::dimension<decltype(getter(std::declval<record_type_t>())), T, cross::non_iterable, H> dim(this, std::get<0>(dimension_filter), std::get<1>(dimension_filter), getter);
  dim.add(0,data.begin(), data.end());
  return dim;
}

template<typename T, typename H>
template<typename V, typename G>
inline
auto filter_impl<T,H>::iterable_dimension(G getter) -> cross::dimension<V, T, cross::iterable, H> {
  auto dimension_filter = add_row_to_filters();
  cross::dimension<V, T, cross::iterable, H> dim(this, std::get<0>(dimension_filter), std::get<1>(dimension_filter), getter);
  dim.add(0, data.begin(), data.end());
  return dim;
}

template<typename T, typename H>
inline
std::vector<uint8_t> & filter_impl<T,H>::mask_for_dimension(std::vector<uint8_t> & mask) const {
  return mask;
}

template<typename T, typename H>
template<typename D>
inline
std::vector<uint8_t> & filter_impl<T,H>::mask_for_dimension(std::vector<uint8_t> & mask,
                                                            const D & dim) const {
  std::bitset<8> b = mask[dim.get_offset()];
  b.set(dim.get_bit_index(), true);

  mask[dim.get_offset()] = b.to_ulong();
  return mask;
}

template<typename T, typename H>
template<typename D, typename ...Ts>
inline
std::vector<uint8_t> & filter_impl<T,H>::mask_for_dimension(std::vector<uint8_t> & mask,
                                                          const D & dim,
                                                          Ts&... dimensions) const {
  std::bitset<8> b = mask[dim.get_offset()];
  b.set(dim.get_bit_index(), true);
  mask[dim.get_offset()] = b.to_ulong();
  return mask_for_dimension(mask, dimensions...);
}

template<typename T, typename H>
template<typename ...Ts>
inline
std::vector<T> filter_impl<T,H>::all_filtered(Ts&... dimensions) const {
  std::vector<uint8_t> mask(filters.size());
  mask_for_dimension(mask, dimensions...);
  // std::vector<T> result;

  // for (std::size_t i = 0; i < data.size(); i++) {
  //   if (filters.zero_except_mask(i, mask))
  //     result.push_back(data[i]);
  // }
  // return result;
  return all_filtered_except_mask(mask);
}
template<typename T, typename H>
inline
std::vector<T> filter_impl<T,H>::all_filtered_except_mask(const std::vector<uint8_t> & mask) const {
  std::vector<T> result;

  for (std::size_t i = 0; i < data.size(); i++) {
    if (filters.zero_except_mask(i, mask))
      result.push_back(data[i]);
  }
  return result;
}

template<typename T, typename H>
template<typename ...Ts>
inline
bool filter_impl<T,H>::is_element_filtered(std::size_t index, Ts&... dimensions) const {
  std::vector<uint8_t> mask(filters.size());
  mask_for_dimension(mask, dimensions...);
  return filters.zero_except_mask(index, mask);
}

template<typename T, typename H>
inline
bool filter_impl<T,H>::is_element_filtered_except_mask(std::size_t index, const std::vector<uint8_t> & mask) const {
  return filters.zero_except_mask(index, mask);
}

template<typename T, typename H>
template<typename... _Args>
inline
std::size_t filter_impl<T,H>::emplace (std::size_t position, _Args &&... args) {
  auto old_data_size = data.size();

  auto new_data_size = 1;

  auto lp = data.begin() + position;
  data.emplace(lp,args...);
  auto & b = data.back();
  auto h = hash(b);
  auto p = hash_table.find(h);
    if(p == hash_table.end())
        hash_table[h] = 1;
    else
        p->second++;

  data_size += new_data_size;
  filters.insert(position,new_data_size,data_size);

  data_iterator nbegin = data.begin() + position;

  data_iterator nend = nbegin;
  std::advance(nend, new_data_size);
#ifdef CROSS_FILTER_USE_THREAD_POOL
  add_signal.emit_in_pool(thread_pool_size,position, nbegin, nend);
  post_add_signal.emit_in_pool(thread_pool_size,new_data_size);
#else
  add_signal(position, nbegin, nend);
  post_add_signal(new_data_size);
#endif
  if(add_group_signal.num_slots() != 0) {
    // FIXME: remove temporary vector
    std::vector<T> tmp;
    tmp.push_back(b);

    std::vector<std::size_t> indexes(tmp.size());
    std::iota(indexes.begin(),indexes.end(),old_data_size);
#ifdef CROSS_FILTER_USE_THREAD_POOL
    add_group_signal.emit_in_pool(thread_pool_size,tmp, indexes, old_data_size, new_data_size);
#else
    add_group_signal(tmp, indexes, old_data_size, new_data_size);
#endif
  }
  return position;
}

template<typename T, typename H>
template<typename Iterator>
inline
filter_impl<T,H> & filter_impl<T,H>::add(std::size_t index, Iterator first, Iterator last,  bool allow_duplicates) {
  // auto logger = spdlog::get("console");
  // if(logger) logger->info("start_add");
  auto old_data_size = size();//data.size();
  auto new_data_size = do_add_range(index, first, last, allow_duplicates);
  // if(logger) logger->info("do_add_range");
  if(new_data_size == 0)
    return *this;

  data_iterator nbegin = data.begin() + index;
  data_iterator nend = nbegin;
  std::advance(nend, new_data_size);

#ifdef CROSS_FILTER_USE_THREAD_POOL
  add_signal.emit_in_pool(thread_pool_size, index, nbegin, nend);
  post_add_signal.emit_in_pool(thread_pool_size, new_data_size);
#else
  add_signal(index, nbegin, nend);
  post_add_signal(new_data_size);
#endif
  // if(logger)   logger->info("post_signal");
  if(add_group_signal.num_slots() != 0) {
    // FIXME: remove temporary vector
    std::vector<record_type_t> tmp(first, last);
    std::vector<std::size_t> indexes(tmp.size());
    std::iota(indexes.begin(),indexes.end(),old_data_size);
#ifdef CROSS_FILTER_USE_THREAD_POOL
    add_group_signal.emit_in_pool(thread_pool_size, tmp, indexes, old_data_size, new_data_size);
#else
    add_group_signal(tmp, indexes, old_data_size, new_data_size);
#endif
  }
  //  mutex.release_internal_lock();
  return *this;
}

template<typename T, typename H>
template<typename C>
inline
typename std::enable_if<!std::is_same<C, T>::value, filter_impl<T,H>&>::type
filter_impl<T,H>::add(const C &new_data,  bool allow_duplicates) {
  auto ds = size(); 
  add(ds,std::begin(new_data),std::end(new_data), allow_duplicates);
  return *this;
}

template<typename T, typename H>
template<bool B>
inline
typename std::enable_if<!std::is_same<H,cross::trivial_hash<T>>::value && B, std::size_t>::type
filter_impl<T,H>::do_add_one(std::size_t pos, const T &new_data,bool allow_duplicates) {
  auto h = hash(new_data);
  auto lp = data.begin() + pos;
  if(allow_duplicates) {
    data.insert(lp,new_data);
    auto p = hash_table.find(h);
    if(p == hash_table.end()) {
      hash_table[h] = 1;
    }
    else {
      p->second++;
    }
  } else {
    if(hash_table.find(h) == hash_table.end()) {
      data.insert(lp,new_data);
      hash_table[h] = 1;
    } else {
      return 0;
    }
  }
  return 1;
}

template<typename T, typename H>
template<bool B>
inline
typename std::enable_if<std::is_same<H,cross::trivial_hash<T>>::value && B, std::size_t>::type
filter_impl<T,H>::do_add_one(std::size_t pos, const T &new_data,bool) {
  if(pos == data.size()) {
    data.push_back(new_data);
  } else {
    auto lp = data.begin() + pos;
    data.insert(lp,new_data);
  }
  return 1;
}

template<typename T, typename H>
template<typename Iterator, bool B>
inline
typename std::enable_if<!std::is_same<H,cross::trivial_hash<T>>::value && B, std::size_t>::type
filter_impl<T,H>::do_add_range(std::size_t index, Iterator first, Iterator last,  bool allow_duplicates) {
  auto new_data_size = std::distance(first, last);
  auto insert_index = data.begin();
  std::advance(insert_index,index);

  if (new_data_size > 0) {
    if(allow_duplicates) {

      std::transform(first, last, std::inserter(data,insert_index),
                     [this](auto v) {
                       auto h = hash(v);
                       auto p = hash_table.find(h);
                       if(p == hash_table.end())
                         hash_table[h] = 1;
                       else
                         p->second++;
                       return v;
                     }
        );
      data_size += new_data_size;
      if(new_data_size > 0) {
        filters.insert(index,new_data_size,data_size);
      }
      return new_data_size;;
    } else {
      std::size_t count = 0;
      for(auto p = first; p != last; ++p) {
        auto h = hash(*p);
        if(hash_table.find(h) == hash_table.end()) {
          data.insert(insert_index++,*p);
          hash_table[h] = 1;
          count++;
        }
      }
      data_size += count;
      new_data_size = count;
      if(new_data_size > 0) {
        filters.insert(index,new_data_size,data_size);
      }
      return count;
    }
  }
  return 0;
}
template<typename T, typename H>
template<typename Iterator, bool B>
inline
typename std::enable_if<std::is_same<H,cross::trivial_hash<T>>::value && B, std::size_t>::type
filter_impl<T,H>::do_add_range(std::size_t index, Iterator first, Iterator last,  bool) {

  auto new_data_size = std::distance(first, last);
  auto insert_index = data.begin();
  std::advance(insert_index,index);

  if (new_data_size > 0) {
    data.insert(insert_index,first,last);
    data_size += new_data_size;
    filters.insert(index,new_data_size,data_size);
    return new_data_size;;
  }
  return 0;
}

template<typename T, typename H>
inline
filter_impl<T,H> & filter_impl<T,H>::add(std::size_t pos, const T &new_data,bool allow_duplicates) {
  auto old_data_size = data.size();
  auto new_data_size = do_add_one(pos,new_data,allow_duplicates);
  if(new_data_size == 0)
    return *this;

  data_size += new_data_size;
  filters.insert(pos,new_data_size,data_size);

  data_iterator nbegin = data.begin() + pos;

  data_iterator nend = nbegin;
  std::advance(nend, new_data_size);
#ifdef CROSS_FILTER_USE_THREAD_POOL
  add_signal.emit_in_pool(thread_pool_size, pos, nbegin, nend);
  post_add_signal.emit_in_pool(thread_pool_size, new_data_size);
#else
  add_signal(pos, nbegin, nend);
  post_add_signal(new_data_size);
#endif
  if(add_group_signal.num_slots() != 0) {
    // FIXME: remove temporary vector
    std::vector<T> tmp;
    tmp.push_back(new_data);

    std::vector<std::size_t> indexes(tmp.size());
    std::iota(indexes.begin(),indexes.end(),old_data_size);
#ifdef CROSS_FILTER_USE_THREAD_POOL
    add_group_signal.emit_in_pool(thread_pool_size, tmp, indexes, old_data_size, new_data_size);
#else
    add_group_signal(tmp, indexes, old_data_size, new_data_size);
#endif
  }

  //  trigger_on_change("dataAdded");
  return *this;
}


template<typename T, typename H>
inline
void filter_impl<T,H>::flip_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                           int dimension_bit_index) {
  filters.flip(index, dimension_offset, dimension_bit_index);
}

template<typename T, typename H>
inline
void filter_impl<T,H>::set_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                                           int dimension_bit_index) {
  filters.set(index, dimension_offset, dimension_bit_index);
}

template<typename T, typename H>
inline
void filter_impl<T,H>::reset_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                                            int dimension_bit_index) {
  filters.reset(index, dimension_offset, dimension_bit_index);
}

template<typename T, typename H>
inline
bool filter_impl<T,H>::get_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                                            int dimension_bit_index) {
  return filters.check(index, dimension_offset, dimension_bit_index);
}

template<typename T, typename H>
inline
void filter_impl<T,H>::flip_bit_for_dimension_st(std::size_t index, std::size_t dimension_offset,
                           int dimension_bit_index) {
  filters.flip_st(index, dimension_offset, dimension_bit_index);
}

template<typename T, typename H>
inline
void filter_impl<T,H>::set_bit_for_dimension_st(std::size_t index, std::size_t dimension_offset,
                                           int dimension_bit_index) {
  filters.set_st(index, dimension_offset, dimension_bit_index);
}

template<typename T, typename H>
inline
void filter_impl<T,H>::reset_bit_for_dimension_st(std::size_t index, std::size_t dimension_offset,
                                            int dimension_bit_index) {
  filters.reset_st(index, dimension_offset, dimension_bit_index);
}

template<typename T, typename H>
inline
bool filter_impl<T,H>::get_bit_for_dimension_st(std::size_t index, std::size_t dimension_offset,
                                            int dimension_bit_index) {
  return filters.check_st(index, dimension_offset, dimension_bit_index);
}

template<typename T, typename H>
inline
auto
filter_impl<T,H>::get_data_iterators_for_indexes(std::size_t low, std::size_t high)
    -> std::tuple<data_iterator, data_iterator> {
  //  reader_lock_t lk(mutex);
  auto begin = data.cbegin();
  auto end = data.cbegin();
  std::advance(begin, low);
  std::advance(end, high);
  if (end > data.cend())
    end = data.cend();
  
  return std::make_tuple<data_iterator, data_iterator>(std::move(begin),
                                                       std::move(end));
}

template<typename T, typename H>
template <bool B>
inline
typename std::enable_if<!std::is_same<H,cross::trivial_hash<T>>::value && B, void>::type
filter_impl<T,H>::update_hash_on_remove(std::size_t index) {
  //writer_lock_t lk(mutex);
  auto h = hash(data[index]);
  auto p = hash_table.find(h);
  if(--(p->second) == 0) {
    hash_table.erase(p);
  }
}

template<typename T, typename H>
template <bool B>
inline
typename std::enable_if<std::is_same<H,cross::trivial_hash<T>>::value && B, void>::type
filter_impl<T,H>::update_hash_on_remove(std::size_t) { }

template<typename T, typename H>
template<typename P>
inline
void filter_impl<T,H>::remove_data(P should_remove, std::size_t first, std::size_t last) {
  // auto logger = spdlog::get("console");
  // if(logger) logger->info("remove start");
  std::vector<int64_t> new_index(data_size);
  
  std::vector<std::size_t> removed;
  //    for (std::size_t index1 = 0, index2 = 0; index1 < data_size; ++index1) {
  auto p = new_index.begin();
  auto p1 = p;
  std::advance(p1,first);
  std::iota(p,p1,std::int64_t(0));
  std::size_t index2 = first;
  for (std::size_t index1 = first; index1 < last; ++index1) {
    if (should_remove(index1)) {
      removed.push_back(index1);
      new_index[index1] = REMOVED_INDEX;
      update_hash_on_remove(index1);
    } else {
      new_index[index1] = index2++;
    }
  }
  // if(logger) logger->info("remove prep1");
  std::advance(p,last);
  p1 = new_index.end();
  std::iota(p,p1,std::int64_t(index2));
  // if(logger) logger->info("remove prep2");
  // Remove all matching records from groups.
#ifdef CROSS_FILTER_USE_THREAD_POOL
  filter_signal.emit_in_pool(thread_pool_size, std::numeric_limits<std::size_t>::max(), -1,
                std::vector<std::size_t>(), removed, true);
  // if(logger)logger->info("remove filter");
  // Update indexes.
  remove_signal.emit_in_pool(thread_pool_size, new_index);
#else
  filter_signal(std::numeric_limits<std::size_t>::max(), -1,
                             std::vector<std::size_t>(), removed, true);
  remove_signal(new_index);
#endif
  // if(logger)logger->info("remove remove_signal");
  // Remove old filters and data by overwriting.
  std::size_t index4 = first;
  for (std::size_t index3 = first; index3 < data_size; ++index3) {
    if (new_index[index3] != REMOVED_INDEX) {
      if (index3 != index4) {
        filters.copy(index4, index3);
        data[index4] = data[index3];
      }
      ++index4;
    }
  }
  // if(logger)logger->info("remove copy data");
  data_size -= removed.size();
  data.resize(data_size);
  // if(logger)logger->info("remove shrink");
  filters.truncate(data_size);
  // if(logger)  logger->info("remove truncate");
}


  // removes all records matching the predicate (ignoring filters).
template<typename T, typename H>
template<typename P>
inline
void filter_impl<T,H>::remove(P predicate) {
  remove_data([&](auto i) { return predicate(data[i], i); }, 0, data_size);
}

template<typename T, typename H>
inline
  // Removes all records that match the current filters
void filter_impl<T,H>::remove() {
  remove_data([&](auto i) { return filters.zero(i); }, 0, data_size );
}

template<typename T, typename H>
inline
void filter_impl<T,H>::remove(std::size_t first, std::size_t last) {
  std::vector<int64_t> new_index(data_size);
  std::vector<std::size_t> removed(last-first);

  std::iota(removed.begin(),removed.end(),first);
  std::iota(new_index.begin(),new_index.begin()+first,0);
  std::fill(new_index.begin()+first, new_index.begin()+last,REMOVED_INDEX);
  std::iota(new_index.begin()+last,new_index.end(),first);


  // Remove all matching records from groups.
#ifdef CROSS_FILTER_USE_THREAD_POOL
  filter_signal.emit_in_pool(thread_pool_size,std::numeric_limits<std::size_t>::max(), -1,
                             std::vector<std::size_t>(), removed, true);
  // Update indexes.
  remove_signal.emit_in_pool(thread_pool_size,new_index);
#else
  filter_signal(std::numeric_limits<std::size_t>::max(), -1,
                             std::vector<std::size_t>(), removed, true);
  // Update indexes.
  remove_signal(new_index);
#endif

  // Remove old filters and data by overwriting.
  std::copy(data.begin()+last,data.end(),data.begin()+first);
  filters.erase(first,last);

  data_size -= removed.size();
  data.resize(data_size);
  //  mutex.unlock();
  //  trigger_on_change("dataRemoved");

}
template<typename T, typename H>
inline
void filter_impl<T,H>::clear() {
  data.clear();
  hash_table.clear();
  clear_signal();
}
template<typename T, typename H>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
inline
auto  filter_impl<T,H>::feature(
    cross::filter<T,H> * base,
    AddFunc  add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_) -> cross::feature<std::size_t,
                                                 decltype(initial_func_()),  this_type_t, true> {
  using reduce_type_t = decltype(initial_func_());

  cross::feature<std::size_t, reduce_type_t, this_type_t, true> g(base,
                                                                  [](const value_type_t& ) { return std::size_t(0);},
                                                                  add_func_,
                                                                  remove_func_,
                                                                  initial_func_);
  std::vector<std::size_t> indexes(data.size());
  std::iota(indexes.begin(),indexes.end(),0);
  g.add(data, indexes, 0,data.size());
  return g;
}

} //namespace impl
} //namespace cross
