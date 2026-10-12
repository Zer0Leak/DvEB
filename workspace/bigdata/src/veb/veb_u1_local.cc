#include "bigdata/veb/veb_u1_local.hh"
#include "bigdata/statistc.hh"

namespace bigdata {
namespace veb {

VebU1Local::VebU1Local(bitscnt_t universe, std::shared_ptr<const Id> id) : VebU1(universe, id) {
}

VebU1Local::~VebU1Local() {

}

std::pair<Natural, std::shared_ptr<ISerializable>> VebU1Local::min() const {
    statistic::thread_sample_set_level(universe_);
    return min_;
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebU1Local::max() const {
    statistic::thread_sample_set_level(universe_);
    return max_;
}


std::pair<bool, std::shared_ptr<ISerializable>> VebU1Local::set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min) {
    statistic::thread_sample_set_level(universe_);
    std::pair<Natural, std::shared_ptr<ISerializable>> ret;
    std::swap(ret, min_);
    min_ = min;
    return {ret.first.is_valid(), ret.second};
}

std::pair<bool, std::shared_ptr<ISerializable>> VebU1Local::set_max(std::pair<Natural, std::shared_ptr<ISerializable>> max) {
    statistic::thread_sample_set_level(universe_);
    std::pair<Natural, std::shared_ptr<ISerializable>> ret;
    std::swap(ret, max_);
    max_ = max;
    return {ret.first.is_valid(), ret.second};
}

}
}
