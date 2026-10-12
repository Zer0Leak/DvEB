#include <stdexcept>
#include <sstream>

#include "bigdata/veb/cluster_remote.hh"
#include "bigdata/veb/veb_client.hh"
#include "bigdata/server.hh"
#include "bigdata/factory.hh"
#include "bigdata/tree_rpc_multicast.hh"

namespace bigdata {
namespace veb {

ClusterRemote::ClusterRemote(bitscnt_t bits, std::shared_ptr<const Id> tree_id, std::shared_ptr<Server> server, std::shared_ptr<Factory> factory, tree_uuid_t uuid)
    : bits_(bits), tree_id_(tree_id), server_(server), factory_(factory), uuid_(uuid) {
}

std::shared_ptr<ITree> ClusterRemote::at(const Natural & index) const {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto id = std::make_shared<const Id>(Id::calculate_child_id(*tree_id_, false, Natural(index)));
    auto tree = std::get<2>(factory_->create(uuid_, bits_, id, server_, Factory::kTreeAuto, Factory::kInstanceClient));
    if (tree->exist()) {
        return tree;
    } else {
        return nullptr;
    }
}

std::shared_ptr<ITree> ClusterRemote::alloc(const Natural & index) {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto id = std::make_shared<const Id>(Id::calculate_child_id(*tree_id_, false, Natural(index)));
    auto tree = std::get<2>(factory_->create(uuid_, bits_, id, server_, Factory::kTreeAuto, Factory::kInstanceClient));
    tree->touch();
    return tree;
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> ClusterRemote::insert(const Natural & index, const Natural & key, std::shared_ptr<ISerializable> value) {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto id = std::make_shared<const Id>(Id::calculate_child_id(*tree_id_, false, Natural(index)));
    auto tree = std::get<2>(factory_->create(uuid_, bits_, id, server_, Factory::kTreeAuto, Factory::kInstanceClient));
    return tree->insert(key, value);
}

std::tuple<bool, std::shared_ptr<ISerializable>, bool> ClusterRemote::remove(const Natural & index, const Natural & key) {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto id = std::make_shared<const Id>(Id::calculate_child_id(*tree_id_, false, Natural(index)));
    auto tree = std::get<2>(factory_->create(uuid_, bits_, id, server_, Factory::kTreeAuto, Factory::kInstanceClient));
    return tree->remove(key);
}

std::pair<bool, std::shared_ptr<ISerializable>> ClusterRemote::search(const Natural & index, const Natural & key) {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto id = std::make_shared<const Id>(Id::calculate_child_id(*tree_id_, false, Natural(index)));
    auto tree = std::get<2>(factory_->create(uuid_, bits_, id, server_, Factory::kTreeAuto, Factory::kInstanceClient));
    return tree->search(key);
}

std::pair<Natural, std::shared_ptr<ISerializable>> ClusterRemote::min(const Natural & index) const {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto id = std::make_shared<const Id>(Id::calculate_child_id(*tree_id_, false, Natural(index)));
    auto tree = std::get<2>(factory_->create(uuid_, bits_, id, server_, Factory::kTreeAuto, Factory::kInstanceClient));
    return tree->min();
}

std::pair<Natural, std::shared_ptr<ISerializable>> ClusterRemote::max(const Natural & index) const {
    if (index.bits() > bits_) {
        throw std::out_of_range("Index of " + std::to_string(static_cast<uint32_t>(index)) + " is out of range in " + std::to_string(bits_) + " bits.");
    }
    auto id = std::make_shared<const Id>(Id::calculate_child_id(*tree_id_, false, Natural(index)));
    auto tree = std::get<2>(factory_->create(uuid_, bits_, id, server_, Factory::kTreeAuto, Factory::kInstanceClient));
    return tree->max();
}

void ClusterRemote::expand_set(std::shared_ptr<ITree> tree) {
    auto id = Id::calculate_child_id(*tree_id_, false, Natural(0));
    // just a sanity check
    if (id != *tree->id()) {
        std::stringstream ss;
        ss << "tree_id " << *tree_id_ << ", child id " << *tree->id() << " calculated_id " << id;
        throw std::runtime_error("Expanding a RemoteCluster, but Id doesn't match. " + ss.str());
    }
}

}
}
