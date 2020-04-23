#ifndef VINCENT_CPP_THREADPOOL_H
#define VINCENT_CPP_THREADPOOL_H

#include <functional>
#include <thread>
#include <vector>
#include <list>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool {
public:
    typedef std::function<void()> callable;
    ThreadPool(size_t maxThreads);
    void execute(const callable &block);
    void close();
    size_t getQueueSize() const { return queue_.size(); }
    size_t getThreadsCount() const { return threads_.size(); }

private:
    class CloseException : public std::exception {};

private:
    std::mutex mutex_;
    std::condition_variable cvEmpty_;
    std::atomic<bool> isClosed_;
    std::list<callable> queue_;
    std::vector<std::thread> threads_;

    void addWorker();
    void putToQueue(const callable &block);
    callable getFromQueue();
};

#include "ThreadPool.cpp"

#endif //VINCENT_CPP_THREADPOOL_H
