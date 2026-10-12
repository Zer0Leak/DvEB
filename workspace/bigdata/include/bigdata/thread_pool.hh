#ifndef BIGDATA_THREAD_POOL_H
#define BIGDATA_THREAD_POOL_H

#include <signal.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <deque>

#include <boost/log/trivial.hpp>

namespace bigdata {

class ThreadPool;

class Worker {
public:
    Worker(ThreadPool &s) : pool_(s) { }
    void operator()();

private:
    ThreadPool &pool_;
};

// the actual thread pool
class ThreadPool {
public:
    ThreadPool(size_t threads) : free_workers_(0), stop_(false)
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        for(size_t i = 0;i<threads;++i) {
            free_workers_++;
            workers_.push_back(std::thread(Worker(*this)));
        }
    }

    ~ThreadPool()
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
        condition_.notify_all();
        lock.unlock();

        for(auto & w : workers_) w.join();
    }


    // add new work item to the pool
    template<class F, typename... Args>
    void enqueue(F f, Args&&... args)
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) return;
        tasks_.push_back(std::bind(f, std::forward<Args>(args)...));
        if (free_workers_ == 0) {
            free_workers_++;
            workers_.push_back(std::thread(Worker(*this)));
            BOOST_LOG_TRIVIAL(debug) << "Workers increased to " << workers_.size();
        }

        BOOST_LOG_TRIVIAL(trace) << "Task size is " << tasks_.size();

        condition_.notify_one();
    }

    //void kill(int sig) {
    //    for(auto & w : workers_) {
    //        pthread_kill(w.native_handle(), sig);
    //    }
    //}

private:
    friend class Worker;

    uint32_t free_workers_;
    std::vector< std::thread > workers_;
    std::deque<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

}

#endif  // BIGDATA_THREAD_POOL_H
