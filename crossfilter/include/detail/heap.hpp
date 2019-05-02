/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef HEAP_H_GUARD
#include <vector>
#include <functional>
#include <algorithm>
template<typename T, typename U>
struct Heap  {
  // Sifts the element a[lo+i-1] down the heap, where the heap is the contiguous
  // slice of array a[lo:lo+n]. This method can also be used to update the heap
  // incrementally, without incurring the full cost of reconstructing the heap.
static  void sift(std::vector<T> & a, std::size_t i, std::size_t n, std::size_t lo, std::function<U(const T&)> f) {
    T d = a[--lo + i];
    U x = f(d);
    std::size_t child;
    while ((child = i << 1) <= n) {
      if (child < n && f(a[lo + child + 1]) < f(a[lo + child])  ) child++;
      if (x < f(a[lo + child]) || x ==  f(a[lo + child])) break;
      a[lo + i] = a[lo + child];
      i = child;
    }
    a[lo + i] = d;
  }

  // Builds a binary heap within the specified array a[lo:hi]. The heap has the
  // property such that the parent a[lo+i] is always less than or equal to its
  // two children: a[lo+2*i+1] and a[lo+2*i+2].
  static std::vector<T> & heap(std::vector<T> &a, std::size_t lo, std::size_t hi, std::function<U(const T&)> f) {
    std::size_t n = hi - lo;

    std::size_t i = (n >> 1) + 1;
    while (--i > 0) sift(a, i, n, lo, f);

    return a;
  }

  // Sorts the specified array a[lo:hi] in descending order, assuming it is
  // already a heap.
static  std::vector<T>&  sort(std::vector<T> & a, std::size_t lo, std::size_t hi, std::function<U(const T&)> f) {
    std::size_t n = hi - lo;
    while (--n > 0)  {
      auto t = a[lo];
      a[lo] = a[lo + n];
      a[lo + n] = t;
      sift(a, 1, n, lo, f);
    }
    return a;
  }

  // Returns a new array containing the top k elements in the array a[lo:hi].
  // The returned array is not sorted, but maintains the heap property. If k is
  // greater than hi - lo, then fewer than k elements will be returned. The
  // order of elements in a is unchanged by this operation.
static  std::vector<T> select(const std::vector<T> & a, std::size_t lo, std::size_t hi,
                              std::size_t  k, std::function<U(const T&)> f) {
    k = std::min(hi - lo, k);
    std::vector<T> queue(k);
    U min;
    std::size_t i = 0;
    T d;

    for (i = 0; i < k; ++i) queue[i] = a[lo++];
    heap(queue, 0, k, f);

    if (lo < hi) {
      min = f(queue[0]);
      do {
        d = a[lo];
        if (min < f(d)) {
          queue[0] = d;
          min = f(heap(queue, 0, k, f)[0]);
        }
      } while (++lo < hi);
    }

    return queue;
  }
};


#endif
