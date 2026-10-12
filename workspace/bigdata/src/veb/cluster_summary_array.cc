#include <stdexcept>

#include "bigdata/veb/cluster_summary_array.hh"
#include "bigdata/veb/veb_summary_local.hh"
#include "bigdata/veb/veb_u1_summary_local.hh"
#include "bigdata/veb/icluster.hh"

namespace bigdata {
namespace veb {

ClusterSummaryArray::ClusterSummaryArray(bitscnt_t bits) : bits_(bits), array_(1ull << bits_) {
}

std::shared_ptr<ITree> ClusterSummaryArray::at(const Natural & index) const {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    return array_[static_cast<uint32_t>(index)];
}

std::shared_ptr<ITree> ClusterSummaryArray::alloc(const Natural & index) {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    if (array_[static_cast<uint32_t>(index)] == NULL) {
        if (bits_ == 1) {
            array_[static_cast<uint32_t>(index)] = std::make_shared<VebU1SummaryLocal>(bits_, nullptr);
        } else {
            array_[static_cast<uint32_t>(index)] = std::make_shared<VebSummaryLocal>(bits_, nullptr);
        }
    }
    return array_[static_cast<uint32_t>(index)];
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> ClusterSummaryArray::insert(const Natural & index, const Natural & key, std::shared_ptr<ISerializable> value) {
    return alloc(index)->insert(key, value);
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> ClusterSummaryArray::remove(const Natural & index, const Natural & key) {
    return alloc(index)->remove(key);
}

std::pair<bool, std::shared_ptr<ISerializable>> ClusterSummaryArray::search(const Natural & index, const Natural & key) {
    auto tree = at(index);
    if (tree != NULL) {
        tree->search(key);
    }
    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> ClusterSummaryArray::min(const Natural & index) const {
    auto tree = at(index);
    if (tree != NULL) {
        tree->min();
    }
    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> ClusterSummaryArray::max(const Natural & index) const {
    auto tree = at(index);
    if (tree != NULL) {
        tree->max();
    }
    return {};
}


void ClusterSummaryArray::expand_set(std::shared_ptr<ITree>) {
    throw std::runtime_error("Can't expand summary cluster.");
}

}
}
