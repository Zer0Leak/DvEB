#include <stdexcept>

#include <boost/log/trivial.hpp>

#include "bigdata/veb/veb_u1.hh"
#include "bigdata/util.hh"
#include "bigdata/statistc.hh"

namespace bigdata {
namespace veb {

VebU1::VebU1(bitscnt_t universe, std::shared_ptr<const Id> id)
    : universe_(universe)
    , id_(id)
{
}

VebU1::~VebU1() {

}

bitscnt_t VebU1::universe() {
    statistic::thread_sample_set_level(universe_);

    return universe_;
}

std::shared_ptr<const Id> VebU1::id() {
    statistic::thread_sample_set_level(universe_);

    return id_;
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> VebU1::insert(const Natural & key,
                                                                      std::shared_ptr<ISerializable> value) {
    statistic::thread_sample_set_level(universe_);
    // BOOST_LOG_TRIVIAL(trace) << std::hex << "Inserting " << key << " at " << *id_ << std::dec << "(" << universe_ << ")";

    std::pair<bool, std::shared_ptr<ISerializable>> ret_data;

    if (UNLIKELY(!key.is_valid())) {
        throw std::runtime_error("Key is not valid.");
    }
    if (!min().first.is_valid()) {
        set_min(std::make_pair(key, value));
        set_max(min());
        return {};
    }

    if (key == min().first) {        // allow replace
        ret_data = set_min(std::make_pair(key, value));
        if (key == max().first) {
            set_max(min());
        }
        return std::make_tuple(ret_data.first, ret_data.second, true);
    } else if (key == max().first) {
        ret_data = set_max(std::make_pair(key, value));
        return std::make_tuple(ret_data.first, ret_data.second, true);
    }

    if (key < min().first) {
        set_min(std::make_pair(key, value));
    } else {  // if (x > max_.first) {
        set_max(std::make_pair(key, value));
    }

    return std::make_tuple<bool, std::shared_ptr<ISerializable>, bool>(false, {}, true);
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> VebU1::remove(const Natural & key) {
    statistic::thread_sample_set_level(universe_);

    std::pair<bool, std::shared_ptr<ISerializable>> ret_data;

    if (min().first.is_valid() == false) {
        return {};
    }

    if (key != min().first && key != max().first) {
        return std::make_tuple<bool, std::shared_ptr<ISerializable>, bool>(false, {}, true);
    }
    if (!key.is_valid()) return std::make_tuple<bool, std::shared_ptr<ISerializable>, bool>(false, {}, true);

    if (min().first == max().first) {
        ret_data = set_min({});
        set_max(min());
        return std::make_tuple(ret_data.first, ret_data.second, true);
    }

    if (key == 0) {
        ret_data = set_min(max());
    } else {
        ret_data = set_max(min());
    }

    return std::make_tuple(ret_data.first, ret_data.second, true);
}

std::pair<bool, std::shared_ptr<ISerializable>> VebU1::search(const Natural & key) {
    statistic::thread_sample_set_level(universe_);

    if (!key.is_valid()) return {};

    if (min().first.is_valid() == false) {
        return {};
    }

    if (key == min().first) {
        return {true, min().second};
    } else if (key == max().first ) {
        return {true, max().second};
    }
    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebU1::successor(const Natural & key) {
    statistic::thread_sample_set_level(universe_);

    if (UNLIKELY(!key.is_valid())) {
        throw std::runtime_error("Key is not valid.");
    }
    if (key == 0 && max().first == 1) {
        return max();
    }
    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebU1::predecessor(const Natural & key) {
    statistic::thread_sample_set_level(universe_);

    if (UNLIKELY(!key.is_valid())) {
        throw std::runtime_error("Key is not valid.");
    }

    if (key == 1 && min().first.is_valid() && min().first == 0) {
        return min();
    }
    return {};
}

void VebU1::expanded(std::shared_ptr<ITree> tree) {
    throw std::runtime_error("Can't call expand on Veb of 2^1 universe size.");
}

std::shared_ptr<ITree> VebU1::summary() const {
    throw std::runtime_error("There is no summary in Veb of 2^1 universe size.");
}

std::shared_ptr<ICluster> VebU1::cluster() const {
    throw std::runtime_error("There is no cluster in Veb of 2^1 universe size.");
}

}
}
