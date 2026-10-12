#include <stdexcept>

#include "bigdata/veb/cluster_summary_hashtable.hh"
#include "bigdata/veb/veb_summary_local.hh"
#include "bigdata/veb/veb_u1_summary_local.hh"

namespace bigdata {
namespace veb {

ClusterSummaryHashTable::ClusterSummaryHashTable(bitscnt_t bits) : bits_(bits) {
}

std::shared_ptr<ITree> ClusterSummaryHashTable::at(const Natural & index) const {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index with " + std::to_string(index.bits()) + "bits is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto it = hashtable_.find(index);
    if (it != hashtable_.end()) {
        return it->second;
    }
    return {};
}

std::shared_ptr<ITree> ClusterSummaryHashTable::alloc(const Natural & index) {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index with " + std::to_string(index.bits()) + "bits is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto it = hashtable_.find(index);
    if (it == hashtable_.end()) {
        if (bits_ == 1) {
            auto r = hashtable_.insert({index, std::make_shared<VebU1SummaryLocal>(bits_, nullptr)});
            return r.first->second;
        } else {
            auto r = hashtable_.insert({index, std::make_shared<VebSummaryLocal>(bits_, nullptr)});
            return r.first->second;
        }
    } else {
        return it->second;
    }
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> ClusterSummaryHashTable::insert(const Natural & index, const Natural & key, std::shared_ptr<ISerializable> value) {
    return alloc(index)->insert(key, value);
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> ClusterSummaryHashTable::remove(const Natural & index, const Natural & key) {
    return alloc(index)->remove(key);
}

std::pair<bool, std::shared_ptr<ISerializable>> ClusterSummaryHashTable::search(const Natural & index, const Natural & key) {
    auto tree = at(index);
    if (tree != NULL) {
        tree->search(key);
    }
    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> ClusterSummaryHashTable::min(const Natural & index) const {
    auto tree = at(index);
    if (tree != NULL) {
        tree->min();
    }
    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> ClusterSummaryHashTable::max(const Natural & index) const {
    auto tree = at(index);
    if (tree != NULL) {
        tree->max();
    }
    return {};
}

void ClusterSummaryHashTable::expand_set(std::shared_ptr<ITree> tree) {
    throw std::runtime_error("Can't expand summary cluster.");
}

}
}
