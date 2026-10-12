
#include <boost/log/trivial.hpp>

#include "bigdata/processor_cheater.hh"
#include "bigdata/ichannel.hh"
#include "bigdata/server.hh"
#include "bigdata/registry.hh"
#include "bigdata/factory.hh"
#include "bigdata/tree_rpc_multicast.hh"
#include "bigdata/veb/iveb_global.hh"
#include "bigdata/util.hh"
#include "bigdata/properties.hh"
#include "bigdata/statistc.hh"

namespace bigdata {

ProcessorCheater::ProcessorCheater(std::shared_ptr<Registry> registry, std::shared_ptr<Factory> factory)
    : registry_(registry), factory_(factory)
{
}

void ProcessorCheater::process(std::vector<uint8_t> buffer, std::shared_ptr<AddressInfo> addr, std::shared_ptr<Server> server) {
    TreeRpc::Method method = TreeRpc::kInvalid;
    ServerDispatchArgs sda;
    swap(sda.addr, addr);
    TreeRpcMulticast::parse(method, sda, buffer);

    std::dynamic_pointer_cast<AddressInfoMulticast>(sda.addr)->addr_.sin_port = htons(sda.port);

    if (method >= TreeRpc::kMethodBegin && method < TreeRpc::kMethodEnd) {
        if (sda.to_root) {
            return;
        } else {
            if (false == registry_->has(sda.uuid, sda.id)) {
                auto id = std::make_shared<bigdata::Id>();
                bigdata::TreeRpcMulticast trpc(sda.universe > 1 ? sda.universe : 1, server, sda.uuid, id, false);

                trpc.reply(sda.uuid, sda.addr, TreeRpc::kNack, sda.transaction, false, false, nullptr, nullptr, nullptr);
                auto addr = std::dynamic_pointer_cast<AddressInfoMulticast>(sda.addr);
                if (addr == NULL) {
                    BOOST_LOG_TRIVIAL(trace) << "We don't own Root " << sda.uuid << " id " << *sda.id << " universe " << sda.universe << " requested in " << sda.transaction;
                } else {
                    BOOST_LOG_TRIVIAL(trace) << "We don't own Root " << sda.uuid << " id " << *sda.id << " universe " << sda.universe << " requested in " << sda.transaction << " from " << inet_ntoa(addr->addr_.sin_addr) << ":" << addr->addr_.sin_port;
                }

                return;
            }
        }
    } else if (method == TreeRpc::kAnswerAllocCheater ) {
        auto id = std::make_shared<bigdata::Id>();
        bigdata::TreeRpcMulticast trpc(sda.universe > 1 ? sda.universe : 1, server, sda.uuid, id, false);

        {

            std::unique_lock<std::recursive_mutex> lg(registry_->mutex());
            if (registry_->has_root(sda.uuid) == false) {
                registry_->sign(sda.uuid, nullptr);
            }
            if (false == registry_->has(sda.uuid, sda.id)) {
                registry_->sign(sda.uuid, sda.id, nullptr);
            } else {
                lg.unlock();
                auto addr = std::dynamic_pointer_cast<AddressInfoMulticast>(sda.addr);
                if (addr == NULL) {
                    BOOST_LOG_TRIVIAL(warning) << "Root " << sda.uuid << " id " << *sda.id << " universe " << sda.universe << " requested in " << sda.transaction << " already registered.";
                } else {
                    BOOST_LOG_TRIVIAL(warning) << "Root " << sda.uuid << " id " << *sda.id << " universe " << sda.universe << " requested in " << sda.transaction << " from " << inet_ntoa(addr->addr_.sin_addr) << ":" << addr->addr_.sin_port << " already registered.";
                }
            }
        }

        trpc.reply(sda.uuid, sda.addr, TreeRpc::kAckF, sda.transaction, false, false, nullptr, nullptr, nullptr);
    }
}


}
