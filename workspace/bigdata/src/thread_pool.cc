
#include <stdexcept>
#include <iostream>

#include <boost/log/trivial.hpp>

#include <boost/thread/thread.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include "bigdata/thread_pool.hh"

namespace bigdata {

void Worker::operator()() {
    BOOST_LOG_SCOPED_THREAD_TAG("ThreadID", boost::this_thread::get_id());

    std::unique_lock<std::mutex> lock(pool_.queue_mutex_);

    while(!pool_.stop_)
    {
        // look for a work item
        while(!pool_.stop_ && pool_.tasks_.empty())
        { // if there are none wait for notification
            BOOST_LOG_TRIVIAL(trace) << "Worker wating for Task";
            pool_.condition_.wait(lock);
        }

        if(pool_.stop_) // exit if the pool is stopped
            break;

        if (pool_.tasks_.size() > 0) {
            // get the task from the queue
            auto task = pool_.tasks_.front();
            pool_.tasks_.pop_front();

            BOOST_LOG_TRIVIAL(trace) << "Task started. Pool size is " << pool_.tasks_.size();

            //if (pool_.tasks_.size()) {
            //    pool_.condition_.notify_one();
            //}

            pool_.free_workers_--;
            lock.unlock();
            try {
                task();
            } catch (const std::runtime_error & e) {
                BOOST_LOG_TRIVIAL(error) << "Got an exception from a task: " << e.what();
            } catch (...) {
                BOOST_LOG_TRIVIAL(error) << "Got an unknown exception from a task";
            }
            BOOST_LOG_TRIVIAL(trace) << "Task finished";
            lock.lock();
            pool_.free_workers_++;
        } else {
            BOOST_LOG_TRIVIAL(trace) << "No Task to work";
        }
    }
}


}
