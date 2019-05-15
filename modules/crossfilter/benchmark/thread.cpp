#include <type_traits>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <future>
#include <vector>
#include <atomic>
#include <random>
#include <string>
// #include "spdlog/spdlog.h"
// #include "spdlog/async.h"
// #include "spdlog/sinks/stdout_sinks.h"
// #include "spdlog/sinks/basic_file_sink.h"
// #include "spdlog/sinks/daily_file_sink.h"
#include "../include/crossfilter.hpp"


template<std::intmax_t resolution>
std::ostream &operator<<(
    std::ostream &stream,
    const std::chrono::duration<
        std::intmax_t,
        std::ratio<std::intmax_t(1), resolution>
    > &duration)
{
    const std::intmax_t ticks = duration.count();
    stream << (ticks / resolution) << '.';
    std::intmax_t div = resolution;
    std::intmax_t frac = ticks;
    for (;;) {
        frac %= div;
        if (frac == 0) break;
        div /= 10;
        stream << frac / div;
    }
    return stream;
}

template<typename Clock, typename Duration>
std::ostream &operator<<(
    std::ostream &stream,
    const std::chrono::time_point<Clock, Duration> &timepoint)
{
    typename Duration::duration ago = timepoint.time_since_epoch();
    return stream << ago;
}

struct Record {
  Record() {}
  // Record(Record &&) = default;
  uint64_t timestamp;
  float d1;
  float d2;
  float d3;
  float d4;
  std::vector<float> v1;
  std::vector<float> v2;
  std::vector<float> v3;
};

struct RandomLogNormal {
  //  std::random_device rd;
  std::mt19937 gen;
  std::normal_distribution<> d;
  RandomLogNormal(double mean, double stddev)
      :gen(),d(mean,stddev) {}
  double operator()()  {
    return d(gen);
  }
};

struct random_uniform_real {
  //  std::random_device rd;
  std::mt19937 gen;
  std::uniform_real_distribution<float> d;
  random_uniform_real(float min, float max)
      :gen(),d(min,max) {}
  float operator()()  {
    return d(gen);
  }
};
struct random_uniform_int {
  //  std::random_device rd;
  std::mt19937 gen;
  std::uniform_int_distribution<int> d;
  random_uniform_int(int min, int max)
      :gen(),d(min,max) {}
  int operator()()  {
    return d(gen);
  }
};
struct Result {
  std::vector<Record> res;
  std::size_t size;
  explicit Result(std::size_t s):res(s),size(0) {}
  Result():size(0) {}
  Result(const Result & r) = default;
  Result(Result && r) = default;
};

struct filter_t {
  cross::filter<Record> filter;
  std::atomic<std::size_t> filter_size;
  filter_t():filter_size(0) {}
};

struct stat_t {
  struct time_stat_t {
    std::size_t min_time = std::numeric_limits<std::size_t>::max();
    std::size_t max_time= 0;
    std::size_t time_sum = 0;
    std::size_t time_cnt = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> min_timestamp;
    std::chrono::time_point<std::chrono::high_resolution_clock> max_timestamp;
    time_stat_t() {
    }
  };
  std::map<std::string,time_stat_t> stat_map;
  std::mutex mutex;
  void update(const std::string & key, std::size_t time) {
    std::lock_guard<std::mutex> lk(mutex);
    auto p = stat_map.find(key);
    if(p == stat_map.end()) {
      p = stat_map.insert(std::make_pair(key,time_stat_t())).first;
    }
    p->second.time_sum += time;
    p->second.time_cnt++;
    if(p->second.min_time > time) {
      p->second.min_time = time;
      p->second.min_timestamp = std::chrono::high_resolution_clock::now();
      // spdlog::get("console")->info("stat set_min for {} =  {}", key,p->second.min_time);
    }
    if(p->second.max_time < time) {
      p->second.max_time = time;
      p->second.max_timestamp = std::chrono::high_resolution_clock::now();
      // spdlog::get("console")->info("stat set_max for {} =  {}", key,p->second.max_time);
    }
  }
  void print() {
    std::lock_guard<std::mutex> lk(mutex);
    // auto logger = spdlog::get(log);
    for(auto & m : stat_map) {
      auto tm = (m.second.time_cnt > 0) ? m.second.time_sum / m.second.time_cnt : 0;

      //      logger->info("stat {} : min = {} ms, avg = {} ms, max = {} ms -- {} {}", m.first, m.second.min_time, tm, m.second.max_time, os1.str(), os2.str());
      if(m.second.min_time == std::numeric_limits<std::size_t>::max())
        m.second.min_time = 0;
      std::cout << "stat " << m.first << " : min = " << m.second.min_time << " ms, avg = " << tm << " ms, max = " << m.second.max_time << std::endl;
      m.second.time_sum = 0;
      m.second.time_cnt = 0;
      m.second.max_time = 0;
      m.second.min_time = std::numeric_limits<std::size_t>::max();
    }
    std::cout << " -------------------------------------------------" << std::endl;
  }
};
struct producer_t {
  filter_t & filter;
  stat_t & stat;
  std::atomic<bool> stop;
  uint32_t max_size = 100000000;
  uint32_t chunk_size = 100000;
  uint64_t time_sum = 0;
  uint64_t time_cnt = 0;
  uint64_t time2_sum = 0;
  uint64_t time2_cnt = 0;
  int min_batch_size = 1000;
  int max_batch_size = 1000;
  explicit producer_t(filter_t & f, stat_t & s):filter(f), stat(s), stop(false) {};
  void run() {
    // spdlog::get("console")->info("profucer");
    random_uniform_real gen(1,1000);
    random_uniform_int r1(1,24);
    random_uniform_int r2(1,48);
    random_uniform_int r3(1,400);
    auto generator= [&gen,&r1,&r2,&r3,this](auto & v) {
                      auto n1 = r1();
                      auto n2 = r2();
                      auto n3 = r3();
                      std::size_t batch_size = min_batch_size;
                      if(n1 + n2 + n3  < 200) {
                        batch_size = max_batch_size;
                      }
                      auto cur = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                      v.res.resize(batch_size);
                      for(auto& r : v.res) {
                        r.timestamp =cur;
                        r.d1 = gen();
                        r.d2 = gen();
                        r.d3 = gen();
                        r.d4 = gen();
                        r.v1.resize(n1);
                        r.v2.resize(n2);
                        r.v3.resize(n3);
                        std::for_each(r.v1.begin(),r.v1.end(),[&gen](auto & v) { v = gen();});
                        std::for_each(r.v2.begin(),r.v2.end(),[&gen](auto & v) { v = gen();});
                        std::for_each(r.v3.begin(),r.v3.end(),[&gen](auto & v) { v = gen();});
                      }
                      v.size = batch_size*(sizeof(Record) + (n1 + n2 + n3)*sizeof(float));
                      return ;
                    };
    Result batch1;
    Result batch2;
    auto fill = [this](Result & b) {
                  b.res.reserve(max_batch_size);
                  for(auto & v : b.res) {
                    v.v1.reserve(24);
                    v.v2.reserve(48);
                    v.v2.reserve(400);
                  }
                  b.size = 0;
                };
    fill(batch1);
    fill(batch2);
    auto feature = std::async(std::launch::async,[&batch1,&generator](){return generator(batch1);});
    std::size_t add_size = 0;
    // auto logger = spdlog::get("console");
    while(!stop) {
      // logger->info("start") ;
      auto start = std::chrono::high_resolution_clock::now();
      feature.get();
      batch1.res.swap(batch2.res);
      batch2.size = batch1.size;
      feature =  std::async(std::launch::async,[&batch1,&generator](){return generator(batch1);});
      auto end = std::chrono::high_resolution_clock::now();
      stat.update("producer_generator",std::chrono::duration_cast<std::chrono::milliseconds>((end - start)).count());
      // logger->info("prepare") ;
      filter.filter.add(batch2.res);
      // logger->info("added {0}",batch2.res.size()) ;
      add_size += batch2.res.size();
      auto fs = filter.filter_size.load();
      filter.filter_size.store(fs + batch2.size);
      auto end2 = std::chrono::high_resolution_clock::now();
      stat.update("producer_add",std::chrono::duration_cast<std::chrono::milliseconds>((end2 - end)).count());
      //      logger->info("prolog");
    }
  } 
};
struct remover_t {
  filter_t & filter;
  stat_t & stat;
  std::atomic<bool> stop;
  std::size_t max_size = 68709120;
  std::mutex mutex;
  std::condition_variable cv;
  bool notified = false;
  explicit remover_t(filter_t & f, stat_t & s, std::size_t ms = 300000):filter(f),stat(s),stop(false),max_size(ms) {};
  void run() {
    filter.filter.onChange([this](cross::event event) {
                             if(event == cross::dataAdded) {
                               if(filter.filter_size.load() >= max_size) {
                                 std::unique_lock<std::mutex> lock(mutex);
                                 notified = true;
                                 cv.notify_one();
                               }
                             }
                           });
    std::unique_lock<std::mutex> lock(mutex);
    // auto logger = spdlog::get("console");
    while(!stop) {
      while(!notified) {
        cv.wait(lock);
      }
      uint32_t max_time = 1000000; // start from 1 second;
      auto cur_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
      while(filter.filter.size() >= max_size) {
        if(filter.filter.size() == 0) {
          filter.filter_size = 0;
          break;
        }
        std::size_t removed = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        // logger->info("remove prepare out");
        filter.filter.remove([&cur_time, &removed, max_time](auto & v, int) {
                               // remove records older than max_time
                               if(cur_time - v.timestamp > max_time) {
                                 removed++;
                                 return true;
                               }
                               return false;
                             });
        // logger->info("remove {} bytes and {} records {}",removed_size, removed, max_time);
        auto t2 = std::chrono::high_resolution_clock::now();
        stat.update("remove",std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count());

        max_time -= (max_time > 500000) ? 500000 : (max_time > 10000) ? 10000 : 1000 ; // decrease max_time for 500 milliseconds;
      }
      notified = false;
    }
  }
};

template<typename D>
struct dim_operation_t {
  D dim;
  typename D::value_type_t top;
  typename D::value_type_t bottom;
  bool flag = false;
  dim_operation_t(dim_operation_t && d):dim(std::move(d.dim)) {}
  dim_operation_t(D && d):dim(std::move(d)) {}
  void operation1() {
    auto t = dim.top(20);
    auto b = dim.bottom(20);
    if(t.empty() || b.empty()) {
      flag = true;
      return;
    }
    if(t[0].d1 < b[0].d1) {
      top  = b[0].d1;
      bottom = t[0].d1;
    } else {
      top  = t[0].d1;
      bottom = b[0].d1;
    }
    flag = false;
  }
  void operation2() {
    if(flag)
      return;
    dim.filter_range(bottom,top);
  }
  void operation3() {
    dim.filter_function([](auto & r) { return r > 50 ;});
  }
  void operation_clear() {
    dim.filter_all();
  }
};

template<typename G>
struct group_operation_t {
  G group;
  group_operation_t(group_operation_t && g):group(std::move(g.group)) {}
  group_operation_t(G && g):group(std::move(g)) {}
  void operation1() {
    group.order([](auto r) {return -r;});
  }
  void operation2() {
    group.top(20,[](auto r) { return r;});
  }
  void operation3() {
    group.top(20);
  }
  void operation_clear() {
    group.order_natural();
  }
};
template<typename Dim>
struct consumer_t {
  filter_t & filter;
  stat_t & stat;
  Dim dim;
  std::mutex mutex;
  std::condition_variable cv;
  bool notified = false;
  std::chrono::time_point<std::chrono::high_resolution_clock> last_time;
  std::string key;
  consumer_t(consumer_t && v) :filter(v.filter),stat(v.stat),dim(std::move(v.dim)),notified(false),key(v.key) {};

  explicit consumer_t(filter_t & f, stat_t & s, int k, Dim && dim):filter(f),stat(s), dim(std::move(dim)) {
    key = std::to_string(k);
  }
  void run() {
    last_time = std::chrono::high_resolution_clock::now();
    // auto logger = spdlog::get("console");
    filter.filter.onChange([this](cross::event event){

                             if(event != cross::dataAdded)
                               return;
                             // logger->info("[{}] onChange {}",key,event);
                             std::unique_lock<std::mutex> lock(mutex);
                             notified = true;
                             cv.notify_one();
                           });
    //    random_uniform_real gen(1,1000);
    std::unique_lock<std::mutex> lock(mutex);

    while(true) {
      while(!notified) {
        cv.wait(lock);
      }
      auto new_time1 = std::chrono::high_resolution_clock::now();
      notified = false;
      stat.update(key + " call_cycle",std::chrono::duration_cast<std::chrono::milliseconds>(new_time1 - last_time).count());
      stat.update(key + " operation1",measure([this](){dim.operation1();}));
      stat.update(key + " operation2",measure([this](){dim.operation2();}));
      stat.update(key + " operation3",measure([this](){dim.operation3();}));
      stat.update(key + " clear",measure([this](){dim.operation_clear();}));
      last_time = std::chrono::high_resolution_clock::now();
    }
  }
  template<typename F>
  uint64_t measure( F && f) {
    auto t1 = std::chrono::high_resolution_clock::now();
    f();
    auto t2 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  }
};
template<typename D>
decltype(auto) make_dimension_consumer(D d, filter_t & filter, stat_t & stat, int key) {
  auto op = dim_operation_t<decltype(d)>(std::move(d));
  return consumer_t<decltype(op)>(filter,stat,key,std::move(op));
}
template<typename G>
decltype(auto) make_group_consumer(G g, filter_t & filter, stat_t & stat, int key) {
  auto gop = group_operation_t<decltype(g)>(std::move(g));
  return consumer_t<decltype(gop)>(filter,stat,key,std::move(gop));
}
int main() {
  //   auto logger = spdlog::stdout_logger_mt("console");
  //  auto logger = spdlog::daily_logger_mt<spdlog::async_factory>("console", "thread_log.txt",10,0);
  //  //  auto logger = spdlog::daily_logger_mt("console", "thread_log.txt",10,0);
  // logger->flush_on(spdlog::level::info);
  // //spdlog::flush_every(std::chrono::seconds(3));
  // spdlog::set_pattern("[%H:%M:%S.%f] [%t]: %v");

  filter_t filter;
#ifdef CROSS_FILTER_USE_THREAD_POOL
  filter.filter.set_thread_pool_size(4);
#endif

  stat_t stat;
  producer_t p(filter,stat);
  remover_t r(filter,stat);

  std::vector<std::thread> threads;

  auto cons1 = make_dimension_consumer(filter.filter.dimension([](auto v) { return v.d1;}),filter,stat,1);
  auto cons2 = make_dimension_consumer(filter.filter.dimension([](auto v) { return v.d2;}),filter,stat,2);
  auto cons3 = make_dimension_consumer(filter.filter.dimension([](auto v) { return v.d3;}),filter,stat,3);
  auto cons4 = make_dimension_consumer(filter.filter.iterable_dimension([](auto v) { return v.v1;}),filter,stat,4);
  auto cons5 = make_dimension_consumer(filter.filter.iterable_dimension([](auto v) { return v.v2;}),filter,stat,5);
  auto cons6 = make_dimension_consumer(filter.filter.iterable_dimension([](auto v) { return v.v3;}),filter,stat,6);

  auto cons7 = make_group_consumer(cons1.dim.dim.feature_count(),filter,stat,7);
  auto cons8 = make_group_consumer(cons1.dim.dim.feature_count(),filter,stat,8);
  auto cons9 = make_group_consumer(cons2.dim.dim.feature_all_count(),filter,stat,9);
  auto cons10 = make_group_consumer(cons3.dim.dim.feature([](auto r)  { return (r < 100) ? 1 : 0; }),filter,stat,10);
  auto cons11 = make_group_consumer(cons4.dim.dim.feature_count(),filter,stat,11);
  auto cons12 = make_group_consumer(cons5.dim.dim.feature_all_count(),filter,stat,12);
  auto cons13 = make_group_consumer(cons6.dim.dim.feature([](auto r)  { return (r < 100.0) ? 1 : 0; }),filter,stat,13);
  auto cons14 = make_group_consumer(filter.filter.feature_count(),filter,stat,14);

  std::thread t1([&r]() { r.run();});
  std::thread t([&p]() { p.run();});
  std::thread stat_thread([&stat]() {
                            while(true) {
                              stat.print();
                              std::this_thread::sleep_for(std::chrono::seconds(10));
                            }
                          });
#define START_THREAD(op)                       \
  threads.emplace_back([&op](){op.run();});

  START_THREAD(cons1);
  START_THREAD(cons2);
  START_THREAD(cons3);
  START_THREAD(cons4);
  START_THREAD(cons5);
  START_THREAD(cons6);
  START_THREAD(cons7);
  START_THREAD(cons8);
  START_THREAD(cons9);
  START_THREAD(cons10);
  START_THREAD(cons11);
  START_THREAD(cons12);
  START_THREAD(cons13);
  START_THREAD(cons14);
  t.join();
  t1.join();
  stat_thread.join();
  for(auto & tt : threads) {
    tt.join();
  }
  return 0;
}
