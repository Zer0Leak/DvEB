
#include <climits>

#include <mutex>
#include <random>

#include <sys/time.h>

#include <boost/log/trivial.hpp>

#include "bigdata/flow_control.hh"
#include "bigdata/properties.hh"

namespace bigdata {

static volatile int32_t last_step_ = 0;
static int32_t consecutive_ok_ = 0;
static std::mutex mutex_;

static std::mt19937 generator_;

uint32_t FlowControl::timeout() {
    std::lock_guard<std::mutex> lkg(mutex_);
    static auto inited = false;
    if (!inited) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        generator_.seed(getpid() ^ tv.tv_usec);

        inited = true;
    }

    uint32_t timeout = Properties::get_instance()->timeout();

    if (last_step_ > 0) {
        timeout *= 1 << last_step_;
        uint32_t val = generator_() % (((timeout / 2) + 1) | 1) ;
        timeout += val;
    }

    return timeout;
}

void FlowControl::timeout_feedback(bool timeout) {
    std::lock_guard<std::mutex> lkg(mutex_);

    if (timeout) {
        if (last_step_ < Properties::get_instance()->retries()) {
            last_step_++;
            BOOST_LOG_TRIVIAL(trace) << "last_step_ incremented to " << last_step_;
        }
        consecutive_ok_ = 0;
    } else {
        consecutive_ok_++;
        if (consecutive_ok_ >= 4) {
            consecutive_ok_ = 4;
            if (last_step_ > 0) {
                last_step_--;
                BOOST_LOG_TRIVIAL(trace) << "last_step_ decremented to " << last_step_;
                consecutive_ok_ = 0;
            }
        }
    }
}

uint32_t FlowControl::long_timeout(bitscnt_t universe) {
    uint32_t expected_jumps = universe > 1 ? log2(universe) : 1;
    expected_jumps *= Properties::get_instance()->jump_multi();  // lets assume there might be a missing at each level

    auto max_step = 1 << Properties::get_instance()->retries();
    uint32_t timeout = Properties::get_instance()->timeout() * max_step;
    timeout += ((timeout / 2) + 1) | 1;
    timeout *= Properties::get_instance()->retries()+1;
    timeout *= expected_jumps;

    return timeout;
}

}
