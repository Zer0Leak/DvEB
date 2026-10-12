#include <stdexcept>

#include "bigdata/veb/cluster_array.hh"
#include "bigdata/veb/veb_local.hh"
#include "bigdata/veb/veb_u1_local.hh"

namespace bigdata {
namespace veb {

ClusterArray::ClusterArray(bitscnt_t bits) : bits_(bits), array_(1ull << bits_) {
}

std::shared_ptr<ITree> ClusterArray::at(const Natural & index) const {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    return array_[static_cast<uint32_t>(index)];
}

std::shared_ptr<ITree> ClusterArray::alloc(const Natural & index) {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    if (array_[static_cast<uint32_t>(index)] == NULL) {
        if (bits_ == 1) {
            array_[static_cast<uint32_t>(index)] = std::make_shared<VebU1Local>(bits_, nullptr);
        } else {
            array_[static_cast<uint32_t>(index)] = std::make_shared<VebLocal>(bits_, nullptr);
        }
    }
    return array_[static_cast<uint32_t>(index)];
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> ClusterArray::insert(const Natural & index, const Natural & key, std::shared_ptr<ISerializable> value) {
    return alloc(index)->insert(key, value);
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> ClusterArray::remove(const Natural & index, const Natural & key) {
    return alloc(index)->remove(key);
}

std::pair<bool, std::shared_ptr<ISerializable>> ClusterArray::search(const Natural & index, const Natural & key) {
    auto tree = at(index);
    if (tree != NULL) {
        tree->search(key);
    }
    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> ClusterArray::min(const Natural & index) const {
    auto tree = at(index);
    if (tree != NULL) {
        tree->min();
    }
    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> ClusterArray::max(const Natural & index) const {
    auto tree = at(index);
    if (tree != NULL) {
        tree->max();
    }
    return {};
}

void ClusterArray::expand_set(std::shared_ptr<ITree> tree) {
    if (array_[0] != NULL) {
        throw std::runtime_error("Can't expand a non empty cluster.");
    }
    array_[0] = tree;
}

}
}
