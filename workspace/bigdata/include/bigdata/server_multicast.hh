
#ifndef BIGDATA_SERVER_MULTICAST_HH_
#define BIGDATA_SERVER_MULTICAST_HH_

#include <cstdint>
#include <cstdio>

#include <memory>
#include <thread>
#include <vector>

#include "bigdata/registry.hh"
#include "bigdata/factory.hh"
#include "bigdata/observer.hh"
#include "bigdata/server.hh"
#include "bigdata/tree_rpc.hh"
#include "bigdata/transaction.hh"
#include "bigdata/thread_pool.hh"

namespace bigdata {

class Registry;
class Factory;
class AddressInfoMulticast;
class Id;
class IProcessor;

class ServerMulticast : public Server {
public:
    ServerMulticast(std::shared_ptr<Registry> registry, std::shared_ptr<Factory> factory, std::shared_ptr<IProcessor> processor);
    ~ServerMulticast();

    void start();
    void stop();
    void join();
    virtual bool exiting() { return cancel_; }

    std::shared_ptr<IChannel> channel() const { return channel_; }

private:
    std::shared_ptr<Registry> registry_;
    std::shared_ptr<Factory> factory_;

    void process(std::vector<uint8_t> buffer, std::shared_ptr<AddressInfo> addr);

    std::vector<uint8_t> buffer_;

    int fd_;
    int fd_event_;
    std::shared_ptr<std::thread> thread_;
    volatile bool cancel_;
    ThreadPool tpool_;
    std::shared_ptr<IChannel> channel_;
    std::shared_ptr<IProcessor> processor_;
};


}

#endif  // BIGDATA_SERVER_MULTICAST_HH_
