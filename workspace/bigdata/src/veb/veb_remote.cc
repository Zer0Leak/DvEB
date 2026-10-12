
#include "bigdata/veb/veb_remote.hh"
#include "bigdata/veb/veb_client.hh"
#include "bigdata/veb/veb_u1_summary_local.hh"
#include "bigdata/veb/cluster_remote.hh"
#include "bigdata/tree_rpc_multicast.hh"
#include "bigdata/server.hh"
#include "bigdata/factory.hh"
#include "bigdata/statistc.hh"

namespace bigdata {
namespace veb {

VebRemote::VebRemote(bitscnt_t universe, std::shared_ptr<const Id> id, tree_uuid_t uuid, std::shared_ptr<Server> server, std::shared_ptr<Factory> factory)
    : Veb(universe, id)
    , uuid_(uuid)
    , server_(server)
    , factory_(factory)
{
    constructor_helper();
}

void VebRemote::constructor_helper() {
    auto id = std::make_shared<Id>(Id::calculate_child_id(*id_, true, Natural(0)));

    summary_ = std::get<2>(factory_->create(uuid_, universe_ >> 1, id, server_, Factory::kTreeAuto, Factory::kInstanceAuto));

    cluster_ = std::make_shared<ClusterRemote>(universe_ >> 1, id_, server_, factory_, uuid_);
}

VebRemote::~VebRemote () {

}

std::pair<Natural, std::shared_ptr<ISerializable>> VebRemote::min() const {
    statistic::thread_sample_set_level(universe_);
    return min_;
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebRemote::max() const {
    statistic::thread_sample_set_level(universe_);
    return max_;
}

void VebRemote::expanded(std::shared_ptr<ITree> tree) {
    statistic::thread_sample_set_level(universe_);
    if (tree == NULL) return;

    auto min = tree->min();

    if (min.first.is_valid()) {
        // remove min from child
        tree->remove(min.first);

        // if child is not empty update summary
        auto child_min = tree->min();
        if (child_min.first.is_valid()) {
            Natural high = child_min.first.high(universe_ >> 1);
            summary_->insert(high, {});
        }

        // set max to chield's max
        max_ = tree->max();
        // set min to child's min
        swap(min_, min);
    }

    // Update the cluster at 0
    // even if there is nothing, lets not allow it to be unallocated
    cluster_->expand_set(tree);
}

std::pair<bool, std::shared_ptr<ISerializable>> VebRemote::set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min) {
    statistic::thread_sample_set_level(universe_);
    std::pair<Natural, std::shared_ptr<ISerializable>> ret;
    std::swap(ret, min_);
    min_ = min;
    return {ret.first.is_valid(), ret.second};
}

std::pair<bool, std::shared_ptr<ISerializable>> VebRemote::set_max(std::pair<Natural, std::shared_ptr<ISerializable>> max) {
    statistic::thread_sample_set_level(universe_);
    std::pair<Natural, std::shared_ptr<ISerializable>> ret;
    std::swap(ret, max_);
    max_ = max;
    return {ret.first.is_valid(), ret.second};
}

std::shared_ptr<ITree> VebRemote::summary() const {
    return  summary_;
}

std::shared_ptr<ICluster> VebRemote::cluster() const {
    return cluster_;
}

}
}
