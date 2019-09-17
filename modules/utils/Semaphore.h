#ifndef VINCENT_CPP_SEMAPHORE_H
#define VINCENT_CPP_SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    Semaphore(int count = 0) : count_(count) {}

    /**
     * Releases one waiting thread.
     */
    inline void notify() {
        std::unique_lock<std::mutex> lock(mtx_);
        ++count_;
        cv_.notify_one();
    }

    /**
     * Wait for notify. Returning true if it was received, false if timeout expired.
     */
    inline bool wait(std::chrono::milliseconds max_duration = std::chrono::milliseconds::max()) {
        std::unique_lock<std::mutex> lock(mtx_);
		if (max_duration == std::chrono::milliseconds::max())
			max_duration = std::chrono::hours(876000);
        while (count_ == 0) {
            if (cv_.wait_for(lock, max_duration) != std::cv_status::no_timeout)
                return false;
        }
        --count_;
        return true;
    }

    /**
     * Returns internal state, for unit tests.
     */
    inline int count() {
        std::unique_lock<std::mutex> lock(mtx_);
        return count_;
    }

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    int count_;
};

#endif //VINCENT_CPP_SEMAPHORE_H
