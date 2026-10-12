

#include <stdexcept>
#include <memory>
#include <tuple>

#include "bigdata/factory.hh"
#include "bigdata/registry.hh"
#include "bigdata/veb/veb_client.hh"
#include "bigdata/veb/veb_remote.hh"
#include "bigdata/veb/veb_local.hh"
#include "bigdata/veb/veb_u1_summary_local.hh"
#include "bigdata/veb/veb_summary_local.hh"
#include "bigdata/veb/veb_u1_local.hh"
#include "bigdata/ichannel.hh"
#include "bigdata/tree_rpc_multicast.hh"

namespace bigdata {

Factory::Factory() {
}

std::tuple<Factory::TreeType, Factory::TreeInstanceType, std::shared_ptr<ITree> >
Factory::create(tree_uuid_t uuid, bitscnt_t universe, std::shared_ptr<const Id> id, std::shared_ptr<Server> server, TreeType tree_type, TreeInstanceType instance_type) {
    if (tree_type == kTreeAuto) {
        tree_type = kTreeVeb;
    }
    if (instance_type == kInstanceAuto) {
        instance_type = kInstanceClient;
    }
    if (instance_type == kInstanceClient) {
        auto tree_rpc = std::make_shared<TreeRpcMulticast>(universe, server, uuid, id, false);
        auto new_tree = std::make_shared<veb::VebClient>(universe, id, tree_rpc);
        return std::make_tuple(tree_type, instance_type, new_tree);
    } else if (instance_type == kInstanceRemote) {
        if (universe == 1) {
            auto summary = Id::is_summary(*id);
            if (summary) {
                return std::make_tuple(tree_type, instance_type, std::make_shared<veb::VebU1SummaryLocal>(universe, id));
            } else {
                return std::make_tuple(tree_type, instance_type, std::make_shared<veb::VebU1Local>(universe, id));
            }
        } else {
            auto new_tree = std::make_shared<veb::VebRemote>(universe, id, uuid, server, this->shared_from_this());
            return std::make_tuple(tree_type, instance_type, new_tree);
        }
    } else if (instance_type == kInstanceLocal) {
        auto summary = Id::is_summary(*id);
        if (summary) {
            if (universe == 1) {
                return std::make_tuple(tree_type, instance_type, std::make_shared<veb::VebU1SummaryLocal>(universe, id));
            } else {
                return std::make_tuple(tree_type, instance_type, std::make_shared<veb::VebSummaryLocal>(universe, id));
            }
        } else {
            if (universe == 1) {
                return std::make_tuple(tree_type, instance_type, std::make_shared<veb::VebU1Local>(universe, id));
            } else {
                return std::make_tuple(tree_type, instance_type, std::make_shared<veb::VebLocal>(universe, id));
            }
        }
    }
    return {};
}

}
