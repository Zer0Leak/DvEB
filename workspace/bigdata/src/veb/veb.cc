#include <memory>
#include <stdexcept>

#include <boost/log/trivial.hpp>

#include "bigdata/factory.hh"
#include "bigdata/registry.hh"
#include "bigdata/veb/icluster.hh"
#include "bigdata/veb/veb.hh"
#include "bigdata/util.hh"
#include "bigdata/statistc.hh"

namespace bigdata {
namespace veb {

Veb::Veb(bitscnt_t universe, std::shared_ptr<const Id> id) : universe_(universe), id_(id) {

}

Veb::~Veb() {

}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> Veb::insert(const Natural & key,
                                                            std::shared_ptr<ISerializable> value) {

    statistic::thread_sample_set_level(universe_);
    // BOOST_LOG_TRIVIAL(trace) << std::hex << "Inserting " << key << " at " << *id_ << std::dec << "(" << universe_ << ")";

    if (UNLIKELY(key.bits() > universe_)) {
        throw std::runtime_error("k.bits()=" + std::to_string(key.bits()) + " is bigger than universe=2^" + std::to_string(universe_));
    }
    if (UNLIKELY(!key.is_valid())) {
        throw std::runtime_error("Key is not valid.");
    }

    std::pair<bool, std::shared_ptr<ISerializable>> ret_data;

    if (min().first.is_valid() == false) {
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
    }

    Natural high;
    Natural low;

    if (key < min().first) {
        high = min().first.high(universe_ >> 1);
        low = min().first.low(universe_ >> 1);
        set_min(std::make_pair(key, value));
    } else {
        high = key.high(universe_ >> 1);
        low = key.low(universe_ >> 1);
    }

    bool there_was_something = false;
    std::tie(ret_data.first, ret_data.second, there_was_something) = cluster()->insert(high, low, value);
    if (false == there_was_something) {
        summary()->insert(high, {});  // if this happen, insert bellow will be constant-time (null min)
    }

    if (key >= max().first) {
        set_max(std::make_pair(key, value));
    }

    return std::make_tuple(ret_data.first, ret_data.second, true);
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> Veb::remove(const Natural & key) {
    statistic::thread_sample_set_level(universe_);

    Natural new_key_aux;
    const Natural * new_key = &key;

    if (UNLIKELY(key.bits() > universe_)) {
        throw std::runtime_error("k.bits()=" + std::to_string(key.bits()) + " is bigger than universe=2^" + std::to_string(universe_));
    }

    if (min().first.is_valid() == false) {
        return {};
    }

    if (key < min().first || key > max().first) {
        return {};
    }

    if (UNLIKELY(!key.is_valid())) {
        //throw std::runtime_error("Key is not valid.");
        return std::make_tuple<bool, std::shared_ptr<ISerializable>, bool>(false, {}, true);
    }

    std::pair<bool, std::shared_ptr<ISerializable>> ret_data;

    if (min().first == max().first) {
        if (key == min().first) {
            ret_data = set_min({});
            set_max(min());
            return std::make_tuple(ret_data.first, ret_data.second, true);
        }
        return std::make_tuple<bool, std::shared_ptr<ISerializable>, bool>(false, {}, true);
    }

    if (key == min().first) {
        auto first_cluster = summary()->min();
        if (first_cluster.first.is_valid()) {
            auto first_offset = cluster()->min(first_cluster.first);
            if (first_offset.first.is_valid()) {
                new_key_aux = (first_cluster.first << (universe_ >> 1)) | first_offset.first;
                ret_data = set_min(std::make_pair(new_key_aux, first_offset.second));
                new_key = &new_key_aux;
            }
        }
    }

    Natural high = new_key->high(universe_ >> 1);
    Natural low = new_key->low(universe_ >> 1);

    bool there_was_something = false;
    if (!ret_data.first) {
        std::tie(ret_data.first, ret_data.second, there_was_something) = cluster()->remove(high, low);
    } else {
        std::tie(std::ignore, std::ignore, there_was_something) = cluster()->remove(high, low);
    }

    if (there_was_something) {
        auto cluter_min = cluster()->min(high);

        if (cluter_min.first.is_valid() == false) {
            summary()->remove(high);
            if (*new_key == max().first) {
                auto summary_max = summary()->max();
                if (!summary_max.first.is_valid()) {
                    set_max(min());
                } else {
                    auto smax_offset = cluster()->max(summary_max.first);
                    if (smax_offset.first.is_valid()) {
                        Natural max_first = (summary_max.first << (universe_ >> 1)) | smax_offset.first;
                        set_max(std::make_pair(max_first, smax_offset.second));
                    }
                }
            }
        } else if (*new_key == max().first) {
            auto max_offset = cluster()->max(high);
            Natural max_first = (high << (universe_ >> 1)) | max_offset.first;
            set_max(std::make_pair(max_first, max_offset.second));
        }
    }

    return std::make_tuple(ret_data.first, ret_data.second, true);
}

std::pair<bool, std::shared_ptr<ISerializable>> Veb::search(const Natural & key) {
    statistic::thread_sample_set_level(universe_);

    if (UNLIKELY(key.bits() > universe_)) {
        throw std::runtime_error("k.bits()=" + std::to_string(key.bits()) + " is bigger than universe=2^" + std::to_string(universe_));
    }
    if (UNLIKELY(!key.is_valid())) {
        // throw std::runtime_error("Key is not valid.");
        return {};
    }

    if (min().first.is_valid() == false) {
        return {};
    }

    if (key < min().first || key > max().first) {
        return {};
    }

    Natural high = key.high(universe_ >> 1);
    Natural low = key.low(universe_ >> 1);

    if (key == min().first ) {
        return {true, min().second};
    } else if (key == max().first ) {
        return {true, max().second};
    }

    return cluster()->search(high, low);
}

std::pair<Natural, std::shared_ptr<ISerializable>> Veb::successor(const Natural & key) {
    statistic::thread_sample_set_level(universe_);

    if (UNLIKELY(key.bits() > universe_)) {
        throw std::runtime_error("k.bits()=" + std::to_string(key.bits()) + " is bigger than universe=2^" + std::to_string(universe_));
    }
    if (UNLIKELY(!key.is_valid())) {
        throw std::runtime_error("Key is not valid.");
    }

    if (min().first.is_valid()) {
        if (key < min().first) {
            return min();
        }
    } else {
        return {};
    }

    if (max().first.is_valid() && key >= max().first) {  // not required but helps reducing rpc steps on sparses trees
        return {};
    }

    Natural high = key.high(universe_ >> 1);
    Natural low = key.low(universe_ >> 1);

    auto max_low = cluster()->max(high);

    if (max_low.first.is_valid() && low < max_low.first) {
        auto successor = cluster()->at(high)->successor(low);
        Natural index = (high << (universe_ >> 1)) | successor.first;
        return {index, successor.second};
    }

    auto successor_cluster = summary()->successor(high);
    if (successor_cluster.first.is_valid()) {
        auto successor = cluster()->min(successor_cluster.first);
        if (successor.first.is_valid()) {
            Natural index = (successor_cluster.first << (universe_ >> 1)) | successor.first;
            return {index, successor.second};
        }
    }

    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> Veb::predecessor(const Natural & key) {
    statistic::thread_sample_set_level(universe_);

    if (UNLIKELY(key.bits() > universe_)) {
        throw std::runtime_error("k.bits()=" + std::to_string(key.bits()) + " is bigger than universe=2^" + std::to_string(universe_));
    }
    if (UNLIKELY(!key.is_valid())) {
        throw std::runtime_error("Key is not valid.");
    }

    if (min().first.is_valid()) {  // not required but helps reducing rpc steps on sparses trees
        if (key <= min().first) {
            return {};
        }
    } else {
        return {};
    }

    if (max().first.is_valid() && key > max().first) {
        return max();
    }

    Natural high = key.high(universe_ >> 1);
    Natural low = key.low(universe_ >> 1);

    auto min_low = cluster()->min(high);

    if (min_low.first.is_valid() && low > min_low.first) {
        auto predecessor = cluster()->at(high)->predecessor(low);
        Natural index = (high << (universe_ >> 1)) | predecessor.first;
        return {index, predecessor.second};
    }

    auto predecessor_cluster = summary()->predecessor(high);
    if (false == predecessor_cluster.first.is_valid()) {
        if (min().first.is_valid() && key > min().first) {
            return min();
        }
    } else {
        auto predecessor = cluster()->max(predecessor_cluster.first);
        if (predecessor.first.is_valid()) {
            Natural index = (predecessor_cluster.first << (universe_ >> 1)) | predecessor.first;
            return {index, predecessor.second};
        }
    }

    return {};
}

bitscnt_t Veb::universe() {
    statistic::thread_sample_set_level(universe_);

    return universe_;
}

std::shared_ptr<const Id> Veb::id() {
    statistic::thread_sample_set_level(universe_);

    return id_;
}

}
}
