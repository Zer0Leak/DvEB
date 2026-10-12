#include <stdexcept>

#include "bigdata/veb/cluster_hashtable.hh"
#include "bigdata/veb/veb_local.hh"
#include "bigdata/veb/veb_u1_local.hh"

namespace bigdata {
namespace veb {

ClusterHashTable::ClusterHashTable(bitscnt_t bits) : bits_(bits) {
}

std::shared_ptr<ITree> ClusterHashTable::at(const Natural & index) const {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index with " + std::to_string(index.bits()) + "bits is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto it = hashtable_.find(index);
    if (it != hashtable_.end()) {
        return it->second;
    }
    return {};
}

std::shared_ptr<ITree> ClusterHashTable::alloc(const Natural & index) {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index with " + std::to_string(index.bits()) + "bits is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto it = hashtable_.find(index);
    if (it == hashtable_.end()) {
        if (bits_ == 1) {
            auto r = hashtable_.insert({index, std::make_shared<VebU1Local>(bits_, nullptr)});
            return r.first->second;
        } else {
            auto r = hashtable_.insert({index, std::make_shared<VebLocal>(bits_, nullptr)});
            return r.first->second;
        }
    } else {
        return it->second;
    }
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> ClusterHashTable::insert(const Natural & index, const Natural & key, std::shared_ptr<ISerializable> value) {
    return alloc(index)->insert(key, value);
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> ClusterHashTable::remove(const Natural & index, const Natural & key) {
    return alloc(index)->remove(key);
}

std::pair<bool, std::shared_ptr<ISerializable>> ClusterHashTable::search(const Natural & index, const Natural & key) {
    auto tree = at(index);
    if (tree != NULL) {
        tree->search(key);
    }
    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> ClusterHashTable::min(const Natural & index) const {
    auto tree = at(index);
    if (tree != NULL) {
        tree->min();
    }
    return {};
}

std::pair<Natural, std::shared_ptr<ISerializable>> ClusterHashTable::max(const Natural & index) const {
    auto tree = at(index);
    if (tree != NULL) {
        tree->max();
    }
    return {};
}

void ClusterHashTable::expand_set(std::shared_ptr<ITree> tree) {
    auto it = hashtable_.find(Natural(0));
    if (it != hashtable_.end()) {
        throw std::runtime_error("Can't expand a non empty cluster.");
    }
    hashtable_.insert({Natural(0), tree});
}

}
}
