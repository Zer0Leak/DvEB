
#include <stdexcept>

#include "bigdata/read_write_sem.hh"

namespace bigdata {

ReadWriteLock::ReadWriteLock() : readers_(0), writers_(0), cancel_(false) {

}

void ReadWriteLock::enter_read() {
    std::unique_lock<std::mutex> lk(mutex_);
    while(writers_ == 0) {
        if (cancel_) {
            throw std::runtime_error("enter_read cancelled.");
        }
        cond_.wait(lk);
    }
    readers_++;
}

void ReadWriteLock::leave_read() {
    std::lock_guard<std::mutex> lk(mutex_);
    readers_--;
    cond_.notify_all();
}

void ReadWriteLock::enter_write() {
    std::unique_lock<std::mutex> lk(mutex_);
    writers_++;
    while(readers_ > 0 || writers_ > 1) {
        if (cancel_) {
            writers_--;
            throw std::runtime_error("enter_read cancelled.");
        }
        cond_.wait(lk);
    }
}

void ReadWriteLock::leave_write() {
    std::lock_guard<std::mutex> lk(mutex_);
    writers_--;
    cond_.notify_all();
}

void ReadWriteLock::cancel() {
    cancel_ = true;
    std::lock_guard<std::mutex> lk(mutex_);
    cond_.notify_all();
}


}
