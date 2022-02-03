#include "ThreadPool.hpp"

#include <iostream>

ThreadPool::ThreadPool(size_t maxThreads) : isClosed_(false)
{
	for (size_t i = 0; i < maxThreads; ++i)
		addWorker();
}

void ThreadPool::close()
{
	{
		std::lock_guard lock(mutex_);
		isClosed_ = true;
		cvEmpty_.notify_all();
	}
	for (auto &t : threads_) {
		t.join();
		// delete t;
	}
}

void ThreadPool::execute(const callable &block)
{
	try {
		putToQueue(block);
	} catch (const CloseException &e) {
		std::cerr << "ThreadPool: execute on closed pool is ignored" << std::endl;
	}
}

void ThreadPool::addWorker()
{
	threads_.push_back(std::thread([this]() {
		while (!isClosed_) {
			try {
				getFromQueue()();
			} catch (const CloseException &x) {
				break;
			} catch (const std::exception &e) {
				std::cerr << "error in ThreadPool worker: " << e.what() << std::endl;
			} catch (...) {
				std::cerr << "unknown error in ThreadPool worker" << std::endl;
			}
		}
	}));
}

void ThreadPool::putToQueue(const callable &block)
{
	std::lock_guard lock(mutex_);
	queue_.push_back(block);
	if (queue_.size() == 1)
		cvEmpty_.notify_all();
}

ThreadPool::callable ThreadPool::getFromQueue()
{
	std::unique_lock lock(mutex_);
	while (!isClosed_ && queue_.size() == 0)
		cvEmpty_.wait(lock);
	if (isClosed_)
		throw CloseException();
	auto block = std::move(queue_.front());
	queue_.pop_front();
	return block;
}
