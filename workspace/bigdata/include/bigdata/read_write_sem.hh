
#ifndef BIGDATA_READ_WRITE_SEM_HH_
#define BIGDATA_READ_WRITE_SEM_HH_

#include <cstdint>

#include <mutex>
#include <condition_variable>


namespace bigdata {

class ReadWriteLock {
public:
    ReadWriteLock();

    void enter_read();
    void leave_read();
    void enter_write();
    void leave_write();

    void cancel();
    void reset() { cancel_ = false; }

private:
    volatile uint32_t readers_;
    volatile uint32_t writers_;
    volatile bool cancel_;

    std::mutex mutex_;
    std::condition_variable cond_;
};


}

#endif  // BIGDATA_READ_WRITE_SEM_HH_
