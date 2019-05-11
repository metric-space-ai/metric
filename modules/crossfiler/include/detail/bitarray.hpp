/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */


#ifndef BITARRAY_H_GUARD
#define BITARRAY_H_GUARD
#include <vector>
#include <tuple>
#include <bitset>
#include <utility>
#include "../detail/thread_policy.hpp"
class BitArray {
  typedef std::vector<uint8_t> index_vec_t;

  std::vector<index_vec_t> bits;
  std::vector<uint8_t> mask;
  std::size_t indexSize = 0;
  mutable cross::thread_policy::mutex_type mutex;
  using read_lock_t = cross::thread_policy::read_lock_t;
  using write_lock_t = cross::thread_policy::write_lock_t;
  int8_t find_first_zero_bit(int8_t value) {

    auto bs = std::bitset<8>(value);
    for ( std::size_t i = 0; i < bs.size(); i++ ) {
      if ( bs[i] == 0 )
        return i;
    }
    return -1;
  }

 public:
  using pos_t  = std::tuple<std::size_t, int>;

  BitArray() {
    bits.push_back(index_vec_t(indexSize));
    mask.push_back(0);
  }

  std::size_t size() const {
    read_lock_t lk(mutex);
    return bits.size();
  }

  std::size_t get_index_size() const {
    read_lock_t lk(mutex);
    return indexSize;
  }
  void resize(std::size_t newSize) {
    write_lock_t lk(mutex);
    for (auto & v : bits) {
      if (v.size() != newSize) {
        v.resize(newSize);
      }
    }
    indexSize = newSize;
  }
  void insert(std::size_t index, std::size_t insert_size, std::size_t new_size) {
    write_lock_t lk(mutex);
    for (auto & v : bits) {
      if(v.empty()) {
        v.resize(new_size);
      } else  if (v.size() != new_size) {
        auto p = v.begin() + index;
        p = v.insert(p,insert_size,0);
      }
    }
    indexSize = new_size;
  }
  void erase(std::size_t first, std::size_t last) {
    write_lock_t lk(mutex);
    for(auto & v : bits) {
      v.erase(v.begin()+first, v.begin()+last);
      indexSize = v.size();
    }
  }
  // return index in bits array and number of bit for coresponding mask
  std::tuple<std::size_t, int> add_row() {
    write_lock_t lk(mutex);
    for (std::size_t i = 0; i < mask.size(); i++) {
      if (mask[i] != 255) {
        int k = find_first_zero_bit(mask[i]);
        if (k >= 0) {
          auto b = std::bitset<8>(mask[i]);
          b.set(k, true);
          mask[i] = b.to_ulong();
          return std::make_tuple<std::size_t, int>(std::move(i), std::move(k));
        }
      }
    }

    // add new mask row
    bits.push_back(index_vec_t(indexSize));
    mask.push_back(1);
    return std::make_tuple<std::size_t, int>(bits.size()-1, 0);
  }
  uint8_t get_mask(std::size_t i) {
    read_lock_t lk(mutex);
    return mask[i];
  }
  void copy(std::size_t dst, std::size_t src) {
    write_lock_t lk(mutex);
    for (auto & v : bits) {
      v[dst] = v[src];
    }
  }

  void truncate(std::size_t newSize) {
    write_lock_t lk(mutex);
    for (auto & v : bits) {
      v.resize(newSize);
    }
  }
  bool zero(std::size_t index) {
    //    read_lock_t lk(mutex);
    for (auto & v : bits) {
      if (v[index] != 0)
        return false;
    }
    return true;
  }
  bool only(std::size_t index, std::size_t offset, int bitIndex) {
    //read_lock_t lk(mutex);
    auto one = std::bitset<8>(0);
    one.flip(bitIndex);

    for (std::size_t i = 0; i < bits.size(); i++) {
      u_int8_t v = (i == offset) ? one.to_ulong() : 0;

      if (bits[i][index] != v)
        return false;
    }
    return true;
  }
  bool zero_except(std::size_t index, std::size_t offset, int bitIndex) {
    //    read_lock_t lk(mutex);
    for (std::size_t i = 0; i < bits.size(); i++) {
      auto bi = bits[i][index];
      
      if (i == offset) {
        if ( bi != 0 ) {
          auto b0 = std::bitset<8>(0);
          b0.set(bitIndex);
          auto b = std::bitset<8>(bi);
          if (b != b0)
            return false;
        }
      } else {
        if (bi != 0)
          return false;
      }
    }
      return true;
  }
  bool zero_except_mask(std::size_t index, const std::vector<uint8_t> & mask) const {
    //    read_lock_t lk(mutex);
    for (std::size_t i = 0; i < bits.size(); i++) {
      auto v = bits[i][index];
      if (v != 0 && (v & mask[i]) != v)
        return false;
    }
    return true;
  }

  bool only_except(std::size_t index, std::size_t offset1, int bitIndex1, std::size_t offset2, int bitIndex2) {
    //    read_lock_t lk(mutex);
    auto zero = std::bitset<8>(0xff);
    zero.flip(bitIndex1);
    auto onlyOne = std::bitset<8>(0);
    onlyOne.flip(bitIndex2);
    for (std::size_t i = 0; i < bits.size(); i++) {
      auto m = bits[i][index];
      if (i == offset1)
        m &= zero.to_ulong();
      if (m != (i == offset2 ? onlyOne.to_ulong() : 0))
        return false;
    }
    return true;
  }
  bool check_st(std::size_t index, std::size_t offset, int bitIndex) const {
    return (bits[offset][index] >> bitIndex) & 1;
  }
  bool check(std::size_t index, std::size_t offset, int bitIndex) {
    read_lock_t lk(mutex);
    return check_st(index, offset, bitIndex);
  }

  void flip_st(std::size_t index, std::size_t offset, int bitIndex) {
    auto b = std::bitset<8>(bits[offset][index]);
    b.flip(bitIndex);
    bits[offset][index] = b.to_ulong();
  }
  void flip(std::size_t index, std::size_t offset, int bitIndex) {
    write_lock_t lk(mutex);
    flip_st(index, offset, bitIndex);
  }

  // void set(std::size_t index, std::size_t offset, int bitIndex, bool newValue = true) {
  //   // auto b = std::bitset<8>(bits[offset][index]);
  //   // b.set(bitIndex, newValue);
  //   // bits[offset][index] = b.to_ulong();
  //   if(newValue)
  //     bits[offset][index] |= 1 << bitIndex;
  //   else
  //     bits[offset][index] &= ~(1 << bitIndex);
  // }
  void set_st(std::size_t index, std::size_t offset, int bitIndex) {
    bits[offset][index] |= 1 << bitIndex;
  }
  void set(std::size_t index, std::size_t offset, int bitIndex) {
    write_lock_t lk(mutex);
    return set_st(index, offset, bitIndex);
  }

  void reset_st(std::size_t index, std::size_t offset, int bitIndex) {
    bits[offset][index] &= ~(1 << bitIndex);
  }
  void reset(std::size_t index, std::size_t offset, int bitIndex) {
    write_lock_t lk(mutex);
    reset_st(index, offset, bitIndex);
  }
};

#endif
