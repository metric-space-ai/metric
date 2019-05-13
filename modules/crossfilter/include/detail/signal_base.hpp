/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef SIGNAL_BASE_H_GUARD
#define  SIGNAL_BASE_H_GUARD
#define USE_NOD_SIGNALS = 1
#include <tuple>
#include <memory>
#include "../detail/thread_policy.hpp"

#if defined  USE_NOD_SIGNALS
#include "../3dparty/nod.hpp"

#ifdef CROSS_FILTER_USE_THREAD_POOL
#define BOOST_ERROR_CODE_HEADER_ONLY
#include <boost/asio.hpp>
#endif

namespace cross {
namespace signals {
template <typename F> using signal = nod::signal<F>;
using connection = nod::connection;
}
#else
#include <boost/signals2.hpp>
namespace signals {
template <typename F> using signal = boost::signals2::signal<F>;
using connection = boost::signals2::connection;
}
#endif
namespace impl {
template <class F, class Tuple, std::size_t... I>
constexpr decltype(auto) apply_impl( F&& f, Tuple&& t, std::index_sequence<I...> )
{
  return f(std::get<I>(std::forward<Tuple>(t))...);
}
template <class F, class Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t)
{
  return apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
                    std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
}

template<typename Signal, typename Connection>
struct MovableSignal {

  std::unique_ptr<Signal> signal;
  //  std::mutex mutex;
  MovableSignal(const MovableSignal &) = delete;
  MovableSignal & operator = (const MovableSignal &) = delete;
  MovableSignal()
      :signal(std::make_unique<Signal>()) {
  }

  MovableSignal(MovableSignal && s) noexcept
      :signal(std::move(s.signal)) {
  }

  MovableSignal & operator = (MovableSignal && s) noexcept {
    if(&s == this)
      return *this;
    std::swap(signal,s.signal);
    return *this;
  }

  template<typename F>
  Connection connect(const F & f) {
    //    std::lock_guard<std::mutex> lk(mutex);
    return signal->connect(f);
  }
  template<typename ...Args>
  void operator()(Args&&... args) {
    if(signal)
      signal->operator()(args...);
  }
#ifdef CROSS_FILTER_USE_THREAD_POOL
  template<typename ...Args>
  void emit_in_pool(uint32_t thread_pool_size, Args&&... args) {
    if(thread_pool_size == 0) {
      this->operator()(args...);
    } else {
      if(signal) {
        boost::asio::thread_pool pool(thread_pool_size);
        auto tpl = std::make_tuple(std::forward<Args>(args)...);
        signal->emit_in_pool([&pool,&tpl](auto & slot) {
                               boost::asio::post(pool,
                                                 [&slot,&tpl] {
                                                   apply(slot,tpl);
                                                 });
                             });
        pool.join();
      }
    }
  }
#endif
  int num_slots() {
    //    std::lock_guard<std::mutex> lk(mutex);
#if defined USE_NOD_SIGNALS
    return signal->slot_count();
#else
    return signal->num_slots();
#endif

  }
};
} // namespace impl
} // namespace cross
#endif
