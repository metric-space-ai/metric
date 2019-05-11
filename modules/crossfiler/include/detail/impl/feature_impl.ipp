/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include <map>
template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
inline
void feature_impl<Key, Reduce, Dimension, is_group_all>::init_slots() {
  auto filter_slot = [this](std::size_t filter_offset, int filter_bit_num,
                           const std::vector<std::size_t> &added,
                           const std::vector<std::size_t> &removed, bool not_filter) {
    update(filter_offset, filter_bit_num, added, removed, not_filter);
  };

  auto add_slot = [this](const value_vec_t &new_data,
                        const std::vector<std::size_t> &new_indexes,
                        std::size_t old_data_size, std::size_t new_data_size) {
    add(new_data, new_indexes, old_data_size, new_data_size);
  };

  auto remove_slot = [this](const std::vector<int64_t> &removed) {
    remove(removed);
  };
  connection_filter = dimension->connect_filter_slot(filter_slot);
  connection_add = dimension->connect_add_slot(add_slot);
  connection_remove =  dimension->connect_remove_slot(remove_slot);
  connection_dimension = dimension->connect_dispose_slot([this]() { dispose();});
  connection_clear = dimension->connect_clear_slot([this]() { clear();});
}

template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
inline
void feature_impl<Key, Reduce, Dimension, is_group_all>::remove(const std::vector<int64_t> &removed) {
  if(is_group_all)
    return;

  std::vector<int> seen_groups(groups.size(),0);
  std::size_t group_num = 0;
  std::size_t j = 0;
  std::size_t remove_counter = 0;
  std::size_t sz = group_index.size();
  
  for(std::size_t i = 0; i < sz; i++) {
    if(removed[i] != REMOVED_INDEX) {
      group_index[j] = group_index[i];
      group_index.for_each(j,[&seen_groups](auto k) { seen_groups[k] = 1;});
      group_num += group_index.size(j);
      j++;
    } else {
      remove_counter++;
    }
  }
  std::vector<group_type_t> old_groups;
  std::swap(old_groups,groups);
  groups.reserve(group_num);
  sz = seen_groups.size();

  for(std::size_t i = 0, k = 0; i < sz; i++) {
    if(seen_groups[i]) {
      seen_groups[i] = k++;
      groups.push_back(old_groups[i]);
    }
  }
  for(std::size_t i = 0; i < j; i++) {
    group_index.set(i,[&seen_groups](auto k) {
        return seen_groups[k];
      });
  }
  group_index.resize(group_index.size() - remove_counter);
}


template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
inline
void feature_impl<Key, Reduce, Dimension, is_group_all>::update_one(std::size_t filter_offset, int filter_bit_index,
                                                                          const std::vector<std::size_t> &added,
                                                                          const std::vector<std::size_t> &removed,
                                                                          bool not_filter) {

  if ((filter_bit_index > 0 && dimension->dimension_offset == filter_offset &&
       dimension->dimension_bit_index == filter_bit_index) ||
      reset_needed)
    return;
  std::for_each(added.begin(), added.end(),
                [this](auto i) {
                  if(dimension->zero_except(i))
                    groups[0].second = add_func(groups[0].second,dimension->get_raw(i),false);
                });
  std::for_each(removed.begin(), removed.end(),
                [this, filter_offset, filter_bit_index, not_filter] (auto i) {
                  if(filter_bit_index < 0) {
                    if(dimension->zero_except(i))
                      groups[0].second = remove_func(groups[0].second,dimension->get_raw(i),not_filter);
                  } else {
                    if(dimension->only_except(i,filter_offset, filter_bit_index))
                      groups[0].second = remove_func(groups[0].second,dimension->get_raw(i),not_filter);
                  }
                });
}

template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
inline
void feature_impl<Key, Reduce, Dimension, is_group_all>::update(std::size_t filter_offset, int filter_bit_index,
                                                                      const std::vector<std::size_t> &added,
                                                                      const std::vector<std::size_t> &removed,
                                                                      bool not_filter) {
  if (is_group_all && isFlatIndex) {
    update_one(filter_offset, filter_bit_index, added, removed, not_filter);
    return;
  }
  bool b1= dimension->dimension_offset == filter_offset;
  bool b2 = dimension->dimension_bit_index == filter_bit_index;
  (void)b1;
  (void)b2;
  bool b = (dimension->dimension_offset == filter_offset &&
            dimension->dimension_bit_index == filter_bit_index);
  if ( b || reset_needed)
    return;

  for(auto i : added) {
    if(dimension->zero_except(i)) {
      auto & elem = dimension->get_raw(i);
      group_index.for_each(i, [this,elem](auto j) {
          auto & g = groups[j];
          g.second = add_func(g.second, elem, false);
        });
    }
  }

  for(auto i : removed) {
    if(filter_bit_index < 0) {
      if(!dimension->zero_except(i))
        continue;
    } else {
      if(!dimension->only_except(i, filter_offset, filter_bit_index))
        continue;
    }
    auto & elem = dimension->get_raw(i);
    group_index.for_each(i, [this,elem,not_filter](auto j) {
        auto & g = groups[j];
        g.second = remove_func(g.second, elem, not_filter);
      });
  }
}

template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
inline
void feature_impl<Key, Reduce, Dimension, is_group_all>::add(const value_vec_t &new_data,
                                                                   const std::vector<std::size_t> &new_data_index,
                                                                   std::size_t old_data_size, std::size_t new_data_size) {
  if (new_data.empty()) {
    group_index.resize(group_index.size() + new_data_size);
    return;
  }
  data_size = new_data.size() + old_data_size;
  std::vector<group_type_t> new_groups;
  //  std::vector<group_index_t> newGroupIndex(groupIndex.size() + newDataSize);
  GroupIndex<isFlatIndex> new_group_index(group_index.size() + new_data_size);
  std::map<key_type_t, std::vector<std::size_t>>  old_indexes;

  auto sz = group_index.size();
  for (std::size_t i = 0; i < sz; i++) {
    group_index.for_each(i,[this, i,&old_indexes](auto k) {
        auto key = groups[k].first;
        old_indexes[key].push_back(i);
      });
  }

  std::size_t i = 0;
  std::size_t i1 = 0;
  std::size_t i2 = 0;
  key_type_t last_key =
      (groups.empty()) ? key(new_data[0]) : groups[0].first;
  auto reduce_value_f = [this](auto & g, std::size_t i) {
    g.second = add_func(g.second, dimension->get_raw(i), true);
    if (!dimension->zero_except(i))
      g.second = remove_func(g.second, dimension->get_raw(i), false);
  };
  auto group_size = groups.size();
  auto new_values_size = new_data.size();

  for (; i1 < group_size && i2 < new_values_size;) {
    auto new_key = key(new_data[i2]);

    if (new_key < groups[i1].first) {
      if (new_groups.empty() || new_groups.back().first < new_key) {
        auto ngroup = std::make_pair(new_key, reduce_type_t());
        new_groups.push_back(ngroup);
        last_key = new_key;
      }
      reduce_value_f(new_groups.back(), new_data_index[i2]);
      new_group_index.setIndex(new_data_index[i2],new_groups.size()-1);

      i2++;
    } else if (new_key > groups[i1].first) {
      new_groups.push_back(groups[i1]);
      auto v = new_groups.size()-1;
      for (auto &j : old_indexes[groups[i1].first]) {
        new_group_index.setIndex(j,v);
      }
      last_key = groups[i1].first;
      i1++;
    } else {
      //        skip new key
      last_key = groups[i1].first;
      new_groups.push_back(groups[i1]);
      auto gindex = new_groups.size() - 1;;

      for (auto j : old_indexes[groups[i1].first]) {
        new_group_index.setIndex(j,gindex);
      }
      reduce_value_f(new_groups.back(), new_data_index[i2]);
      new_group_index.setIndex(new_data_index[i2],gindex);
      i1++;
      i2++;
    }
  }
  for (; i1 < group_size; i1++, i++) {
    new_groups.push_back(groups[i1]);
    auto gindex = new_groups.size() - 1;

    for (auto &j : old_indexes[groups[i1].first]) {
      new_group_index.setIndex(j,gindex);
    }
  }
  for (; i2 < new_values_size; i2++, i++) {
    auto new_key = key(new_data[i2]);
    if (new_groups.empty() || new_key > last_key) {
      new_groups.push_back(std::make_pair(new_key, reduce_type_t()));
      last_key = new_key;
    }
    new_group_index.setIndex(new_data_index[i2],new_groups.size() - 1);
    auto & g = new_groups[new_groups.size()-1];
    reduce_value_f(g, new_data_index[i2]);
  }
  std::swap(groups, new_groups);
  std::swap(group_index, new_group_index);
}

template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
inline
std::vector<typename feature_impl<Key, Reduce, Dimension, is_group_all>::group_type_t>
feature_impl<Key, Reduce, Dimension, is_group_all>::top(std::size_t k) noexcept {
  auto &t = all();
  auto top = select_func(t, 0, groups.size(), k);
  return sort_func(top, 0, top.size());
}

template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
template<typename OrderFunc>
inline
std::vector<typename feature_impl<Key, Reduce, Dimension, is_group_all>::group_type_t>
feature_impl<Key, Reduce, Dimension, is_group_all>::top(std::size_t k, OrderFunc value) noexcept {
  auto &t = all();
  using T = typename std::decay< decltype(value(std::declval<reduce_type_t>())) >::type;
  auto select = std::bind(Heap<group_type_t, T>::select,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3,
                          std::placeholders::_4,
                          [&value](const group_type_t& g) { return value(g.second);});

  auto sort = std::bind(Heap<group_type_t, T>::sort, std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3,
                        [&value](const group_type_t& g) { return value(g.second);});

  auto top = select(t, 0, groups.size(), k);
  return sort(top, 0, top.size());
}

template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
inline
void feature_impl<Key, Reduce, Dimension, is_group_all>::reset_one() {
  if (groups.empty())
      groups.push_back(group_type_t());

  group_type_t &g = groups[0];
  g.second = initial_func();
  auto group_index_size = group_index.size();
  for (std::size_t i = 0; i < group_index_size; ++i) {
    g.second = add_func(g.second, dimension->get_raw(i), true);
  }
  for (std::size_t i = 0; i < group_index_size; ++i) {
    if (!dimension->zero_except(i)) {
      g.second =
          remove_func(g.second, dimension->get_raw(i), false);
    }
  }
}

template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
inline
void feature_impl<Key, Reduce, Dimension, is_group_all>::reset() {
  if (is_group_all && isFlatIndex) {
    reset_one();
    return;
  }
  // Reset all group values.
  for(auto &g: groups) {
    g.second = initial_func();
  }
  auto group_index_size = group_index.size();
  for (std::size_t i = 0; i < group_index_size; ++i) {
    auto & elem = dimension->get_raw(i);
    bool zero_except = dimension->zero_except(i);
    group_index.for_each(i,[this,elem,zero_except](auto k) {
        auto &g = groups[k];
        g.second = add_func(g.second, elem, true);
        if (!zero_except) {
          g.second =  remove_func(g.second, elem, false);
        }
      });
  }
}


template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
inline
std::vector<typename feature_impl<Key, Reduce, Dimension, is_group_all>::group_type_t> &
feature_impl<Key, Reduce, Dimension, is_group_all>::all() noexcept {
  if (reset_needed) {
    reset();
    reset_needed = false;
  }
  return groups;
}

template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
inline
typename feature_impl<Key, Reduce, Dimension, is_group_all>::reduce_type_t
feature_impl<Key, Reduce, Dimension, is_group_all>::value() noexcept {
  if (reset_needed) {
    reset();
    reset_needed = false;
  }
  if (groups.empty())
    return initial_func();

  return groups[0].second;
}

template <typename Key, typename Reduce, typename Dimension,
          bool is_group_all>
template<typename OrderFunc>
inline
feature_impl<Key, Reduce, Dimension, is_group_all> &
feature_impl<Key, Reduce, Dimension, is_group_all>::order(OrderFunc value) noexcept {
  //  using ph = std::placeholders;
  using T = typename std::decay< decltype(value(std::declval<reduce_type_t>())) >::type;
  select_func = std::bind(Heap<group_type_t, T>::select,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3,
                          std::placeholders::_4,
                          [&value](const group_type_t& g) { return value(g.second);});

  sort_func = std::bind(Heap<group_type_t, T>::sort, std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3,
                        [&value](const group_type_t& g) { return value(g.second);});
  return *this;
}
