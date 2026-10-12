
#ifndef BIGDATA_CHANNEL_MULTICAST_HH_
#define BIGDATA_CHANNEL_MULTICAST_HH_

#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstdint>

#include <vector>
#include <mutex>
#include <condition_variable>

#include "bigdata/ichannel.hh"

namespace bigdata {

class AddressInfoMulticast : public AddressInfo {
public:
    explicit AddressInfoMulticast(const struct sockaddr_in * addr);
    AddressInfoMulticast(const AddressInfoMulticast &);
    AddressInfoMulticast(AddressInfoMulticast &&);
    AddressInfoMulticast & operator=(const AddressInfoMulticast &);
    struct sockaddr_in addr_;
};

class ChannelMulticast : public IChannel {
public:
    ChannelMulticast();
    virtual ~ChannelMulticast();

    virtual void open();
    virtual void close();

    virtual operator FILE*();
    virtual void sync(std::shared_ptr<const AddressInfo> unicast_addr = NULL, bool force_min_size = false);
    virtual std::recursive_mutex & mutex();

    virtual const uint8_t * mac_address() const {
        return mac_address_;
    }

    const AddressInfoMulticast & src_addr() { return src_addr_; }

private:
    std::recursive_mutex mutex_app_;
    std::vector<uint8_t> buffer_;
    int fd_;
    struct sockaddr_in addr_;
    AddressInfoMulticast src_addr_;
    FILE * file_;

    void get_mac(int sock);
    void process(const AddressInfoMulticast * unicast_addr, bool force_min_size);

    uint8_t mac_address_[6];
};

}

#endif  // BIGDATA_CHANNEL_MULTICAST_HH_
