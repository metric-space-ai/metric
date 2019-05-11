/*This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2018 Dmitry Vinokurov */

#ifndef THREAD_POLICY_H_GUARD
#define THREAD_POLICY_H_GUARD

#ifdef CROSS_FILTER_MULTI_THREAD
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#endif 

#if !defined CROSS_FILTER_MULTI_THREAD
// use thread pool only if multithreading enabled
#undef CROSS_FILTER_USE_THREAD_POOL
#endif

// #if defined CROSS_FILTER_USE_THREAD_POOL
//   #if !defined CROSS_FILTER_THREAD_POOL_SIZE
//      #define CROSS_FILTER_THREAD_POOL_SIZE 4
//   #endif
// #endif
namespace cross {
struct singlethread_policy {
  struct mutex_type {
    void lock() {};
    void unlock() {}
    void lock_shared() {}
    void unlock_shared() {}
    void upgrade_to_internal_lock() {}
    void release_internal_lock() {}
    void lock_internal() {}
    void unlock_internal() {}
  };
  struct write_lock_t {
    explicit write_lock_t(const mutex_type &) {}
  };
  struct read_lock_t {
    explicit read_lock_t(const mutex_type &) {}
  };
};

#ifdef CROSS_FILTER_MULTI_THREAD
struct multithread_policy {
  using mutex_type = std::shared_timed_mutex;
  using write_lock_t = std::unique_lock<mutex_type>;
  using read_lock_t = std::shared_lock<mutex_type>;
};
using thread_policy = multithread_policy;
#else
using thread_policy = singlethread_policy;
#endif
}
#endif

