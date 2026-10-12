
#ifndef BIGDATA_PROCESSOR_NODE_HH_
#define BIGDATA_PROCESSOR_NODE_HH_

#include <mutex>
#include <condition_variable>
#include <unordered_map>

#include "bigdata/iprocessor.hh"
#include "bigdata/observer.hh"
#include "bigdata/tree_rpc.hh"

namespace bigdata {

class Registry;
class Server;
class Factory;
class ServerDispatchArgs;
class TreeRpcMulticast;

class ProcessorNode : public IProcessor {
public:
    ProcessorNode(std::shared_ptr<Registry> registry, std::shared_ptr<Factory> factory);
    virtual ~ProcessorNode() = default;

    virtual void process(std::vector<uint8_t> buffer, std::shared_ptr<AddressInfo> addr, std::shared_ptr<Server> server);

private:
    std::shared_ptr<Registry> registry_;
    std::shared_ptr<Factory> factory_;
    TransactionSet active_tranasctions_;

    void reply_ack(TreeRpcMulticast & trpc, const ServerDispatchArgs & sda, std::shared_ptr<Server> server, TreeRpc::Method method, bool has);


    void process_avail(bool active_transaction, std::shared_ptr<Server> server, const ServerDispatchArgs & sda, std::mutex & mutexf, std::condition_variable & cond, volatile bool & ackf_received);
    void process_alloc(bool active_transaction, std::shared_ptr<Server> server, const ServerDispatchArgs & sda, std::mutex & mutexf, std::condition_variable & cond, volatile bool & ackf_received);
    void process_method(bool active_transaction, std::shared_ptr<Server> server, const TreeRpc::Method method, const ServerDispatchArgs & sda, std::mutex & mutexf, std::condition_variable & cond, volatile bool & ackf_received);

    bool process_send_cheater(std::shared_ptr<Server> server, const ServerDispatchArgs & sda, std::mutex & mutexf, std::condition_variable & cond);
};

}

#endif  // BIGDATA_PROCESSOR_NODE_HH_
