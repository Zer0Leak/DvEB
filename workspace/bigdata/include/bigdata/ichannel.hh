
#ifndef BIGDATA_ICHANNEL_HH_
#define BIGDATA_ICHANNEL_HH_

#include <cstdio>

#include <mutex>
#include <memory>

namespace bigdata {

class AddressInfo {
public:
    virtual ~AddressInfo() = default;
};

class IChannel {
public:
    virtual ~IChannel() = default;

    virtual void open() = 0;
    virtual void close() = 0;

    virtual operator FILE*() = 0;
    virtual void sync(std::shared_ptr<const AddressInfo> addr_info = NULL, bool force_min_size = false) = 0;
    virtual std::recursive_mutex & mutex() = 0;
    virtual const uint8_t * mac_address() const = 0;
};

}

#endif  // BIGDATA_ICHANNEL_HH_
