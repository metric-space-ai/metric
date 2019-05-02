/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef DUAL_PIVOT_SORT2_H_GUARD
#define DUAL_PIVOT_SORT2_H_GUARD
#include <vector>
#include <functional>
#include <algorithm>
struct DualPivotsort2 {
  template<typename T, typename F>
  static void insertionsort(std::vector<T> &a, std::size_t lo, std::size_t hi, std::function<F(T&)> f) {
    for (auto i = lo + 1; i < hi; ++i) {
      T t;
      F x;
      std::size_t j = 0;
      for (j = i, t = a[i], x = f(t); j > lo && f(a[j - 1]) > x; --j) {
        a[j] = a[j - 1];
      }
      a[j] = t;
    }
    return;
  }

  template<typename T, typename F>
  static void quicksort(std::vector<T> &a, std::size_t lo, std::size_t hi, std::function<F(T&)> f) {
    if (hi - lo < 32) {
      // auto beg = a.begin();
      // auto end = beg;
      // std::advance(beg, lo);
      // std::advance(end, hi);
      // std::sort(beg, end, [&f](auto lhs, auto rhs) { return f(lhs) < f(rhs);});
      insertionsort(a, lo, hi, f);
      return;
    }
    // Compute the two pivots by looking at 5 elements.
    auto sixth = (hi - lo) / 6;

    auto i1 = lo + sixth;
    auto i5 = hi - 1 - sixth;
    auto i3 = (lo + hi - 1) >> 1;  // The midpoint.
    auto i2 = i3 - sixth;
    auto i4 = i3 + sixth;

    auto e1 = a[i1];
    auto x1 = f(e1);
    auto e2 = a[i2];
    auto x2 = f(e2);
    auto e3 = a[i3];
    auto x3 = f(e3);
    auto e4 = a[i4];
    auto x4 = f(e4);
    auto e5 = a[i5];
    auto x5 = f(e5);

    //    var t;

    // Sort the selected 5 elements using a sorting network.
    if (x1 > x2) {std::swap(e1, e2); std::swap(x1, x2);}
    if (x4 > x5) {std::swap(e4, e5); std::swap(x4, x5);}
    if (x1 > x3) {std::swap(e1, e3); std::swap(x1, x3);}
    if (x2 > x3) {std::swap(e2, e3); std::swap(x2, x3);}
    if (x1 > x4) {std::swap(e1, e4); std::swap(x1, x4);}
    if (x3 > x4) {std::swap(e3, e4); std::swap(x3, x4);}
    if (x2 > x5) {std::swap(e2, e5); std::swap(x2, x5);}
    if (x2 > x3) {std::swap(e2, e3); std::swap(x2, x3);}
    if (x4 > x5) {std::swap(e4, e5); std::swap(x4, x5);}
    auto pivot1 = e2;
    auto pivot2 = e4;
    auto pivotValue1 = x2;
    auto pivotValue2 = x4;

    // e2 and e4 have been saved in the pivot variables. They will be written
    // back, once the partitioning is finished.
    a[i1] = e1;
    a[i2] = a[lo];
    a[i3] = e3;
    a[i4] = a[hi - 1];
    a[i5] = e5;

    auto  less = lo + 1;   // First element in the middle partition.
    auto great = hi - 2;  // Last element in the middle partition.

    // Note that for value comparison, <, <=, >= and > coerce to a primitive via
    // Object.prototype.valueOf; == and === do not, so in order to be consistent
    // with natural order (such as for Date objects), we must do two compares.
    bool pivotsEqual = pivotValue1 <= pivotValue2 && pivotValue1 >= pivotValue2;
    if (pivotsEqual) {
      // Degenerated case where the partitioning becomes a dutch national flag
      // problem.
      //
      // [ |  < pivot  | == pivot | unpartitioned | > pivot  | ]
      //  ^             ^          ^             ^            ^
      // left         less         k           great         right
      //
      // a[left] and a[right] are undefined and are filled after the
      // partitioning.
      //
      // Invariants:
      //   1) for x in ]left, less[ : x < pivot.
      //   2) for x in [less, k[ : x == pivot.
      //   3) for x in ]great, right[ : x > pivot.
      for (auto k = less; k <= great; ++k) {
        auto ek = a[k];
        auto xk = f(ek);
        if (xk < pivotValue1) {
          if (k != less) {
            a[k] = a[less];
            a[less] = ek;
          }
          ++less;
        } else if (xk > pivotValue1) {
          // Find the first element <= pivot in the range [k - 1, great] and
          // put [:ek:] there. We know that such an element must exist:
          // When k == less, then el3 (which is equal to pivot) lies in the
          // interval. Otherwise a[k - 1] == pivot and the search stops at k-1.
          // Note that in the latter case invariant 2 will be violated for a
          // short amount of time. The invariant will be restored when the
          // pivots are put into their final positions.
          /* eslint no-constant-condition: 0 */
          while (true) {
            auto greatValue = f(a[great]);
            if (greatValue > pivotValue1) {
              great--;
              // This is the only location in the while-loop where a new
              // iteration is started.
              continue;
            } else if (greatValue < pivotValue1) {
              // Triple exchange.
              a[k] = a[less];
              a[less++] = a[great];
              a[great--] = ek;
              break;
            } else {
              a[k] = a[great];
              a[great--] = ek;
              // Note: if great < k then we will exit the outer loop and fix
              // invariant 2 (which we just violated).
              break;
            }
          }
        }
      }
    } else {
      // We partition the list into three parts:
      //  1. < pivot1
      //  2. >= pivot1 && <= pivot2
      //  3. > pivot2
      //
      // During the loop we have:
      // [ | < pivot1 | >= pivot1 && <= pivot2 | unpartitioned  | > pivot2  | ]
      //  ^            ^                        ^              ^             ^
      // left         less                     k              great        right
      //
      // a[left] and a[right] are undefined and are filled after the
      // partitioning.
      //
      // Invariants:
      //   1. for x in ]left, less[ : x < pivot1
      //   2. for x in [less, k[ : pivot1 <= x && x <= pivot2
      //   3. for x in ]great, right[ : x > pivot2
      //      (function () { // isolate scope
      for (auto k = less; k <= great; k++) {
        auto ek = a[k]; auto xk = f(ek);
        if (xk < pivotValue1) {
          if (k != less) {
            a[k] = a[less];
            a[less] = ek;
          }
          ++less;
        } else {
          if (xk > pivotValue2) {
            while (true) {
              auto greatValue = f(a[great]);
              if (greatValue > pivotValue2) {
                great--;
                if (great < k) break;
                // This is the only location inside the loop where a new
                // iteration is started.
                continue;
              } else {
                // a[great] <= pivot2.
                if (greatValue < pivotValue1) {
                  // Triple exchange.
                  a[k] = a[less];
                  a[less++] = a[great];
                  a[great--] = ek;
                } else {
                  // a[great] >= pivot1.
                  a[k] = a[great];
                  a[great--] = ek;
                }
                break;
              }
            }
          }
        }
      }
      //      })(); // isolate scope
    }

    // Move pivots into their final positions.
    // We shrunk the list from both sides (a[left] and a[right] have
    // meaningless values in them) and now we move elements from the first
    // and third partition into these locations so that we can store the
    // pivots.
    a[lo] = a[less - 1];
    a[less - 1] = pivot1;
    a[hi - 1] = a[great + 1];
    a[great + 1] = pivot2;

    // The list is now partitioned into three partitions:
    // [ < pivot1   | >= pivot1 && <= pivot2   |  > pivot2   ]
    //  ^            ^                        ^             ^
    // left         less                     great        right

    // Recursive descent. (Don't include the pivot values.)
    quicksort(a, lo, less - 1, f);
    quicksort(a, great + 2, hi, f);

    if (pivotsEqual) {
      // All elements in the second partition are equal to the pivot. No
      // need to sort them.
      return;
    }

    // In theory it should be enough to call _doSort recursively on the second
    // partition.
    // The Android source however removes the pivot elements from the recursive
    // call if the second partition is too large (more than 2/3 of the list).
    if (less < i1 && great > i5) {
      //      (function () { // isolate scope
      F lessValue;
      F greatValue;
      while ((lessValue = f(a[less])) <= pivotValue1 && lessValue >= pivotValue1) ++less;
      while ((greatValue = f(a[great])) <= pivotValue2 && greatValue >= pivotValue2) --great;

      // Copy paste of the previous 3-way partitioning with adaptions.
      //
      // We partition the list into three parts:
      //  1. == pivot1
      //  2. > pivot1 && < pivot2
      //  3. == pivot2
      //
      // During the loop we have:
      // [ == pivot1 | > pivot1 && < pivot2 | unpartitioned  | == pivot2 ]
      //              ^                      ^              ^
      //            less                     k              great
      //
      // Invariants:
      //   1. for x in [ *, less[ : x == pivot1
      //   2. for x in [less, k[ : pivot1 < x && x < pivot2
      //   3. for x in ]great, * ] : x == pivot2
      for (auto k = less; k <= great; k++) {
        auto ek = a[k];
        auto xk = f(ek);
        if (xk <= pivotValue1 && xk >= pivotValue1) {
          if (k != less) {
            a[k] = a[less];
            a[less] = ek;
          }
          less++;
        } else {
          if (xk <= pivotValue2 && xk >= pivotValue2) {
            /* eslint no-constant-condition: 0 */
            while (true) {
              greatValue = f(a[great]);
              if (greatValue <= pivotValue2 && greatValue >= pivotValue2) {
                great--;
                if (great < k) break;
                // This is the only location inside the loop where a new
                // iteration is started.
                continue;
              } else {
                // a[great] < pivot2.
                if (greatValue < pivotValue1) {
                  // Triple exchange.
                  a[k] = a[less];
                  a[less++] = a[great];
                  a[great--] = ek;
                } else {
                  // a[great] == pivot1.
                  a[k] = a[great];
                  a[great--] = ek;
                }
                break;
              }
            }
          }
        }
      }
    }

    // The second partition has now been cleared of pivot elements and looks
    // as follows:
    // [  *  |  > pivot1 && < pivot2  | * ]
    //        ^                      ^
    //       less                  great
    // Sort the second partition using recursive descent.

    // The second partition looks as follows:
    // [  *  |  >= pivot1 && <= pivot2  | * ]
    //        ^                        ^
    //       less                    great
    // Simply sort it by recursive descent.

    quicksort(a, less, great + 1, f);
  }

  //  return sort;
};

#endif
