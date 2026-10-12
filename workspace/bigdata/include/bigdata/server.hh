
#ifndef BIGDATA_SERVER_HH_
#define BIGDATA_SERVER_HH_

#include "bigdata/observer.hh"

#include "bigdata/globals.hh"
#include "bigdata/id.hh"
#include "bigdata/transaction.hh"
#include "bigdata/tree_rpc.hh"

namespace bigdata {

class ITree;
class IChannel;
class AddressInfo;

namespace statistic {
    struct Sample;
}

struct ServerDispatchArgs
{
    tree_uuid_t uuid;
    std::shared_ptr<const Id> id;
    Natural key;
    Id expand_tree_id;
    bool to_root = false;
    Transaction transaction;
    bool has;
    bool had_something;
    bitscnt_t universe;
    std::shared_ptr<AddressInfo> addr;
    uint16_t port;
    std::shared_ptr<statistic::Sample> sample;
};

class Server : public Observable<TreeRpc::Method, ServerDispatchArgs &> {
public:
    virtual ~Server() = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void join() = 0;
    virtual bool exiting() = 0;

    virtual std::shared_ptr<IChannel> channel() const = 0;
};

}


#endif  // BIGDATA_SERVER_HH_
