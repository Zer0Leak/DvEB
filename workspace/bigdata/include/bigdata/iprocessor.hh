
#ifndef BIGDATA_IPROCESSOR_HH_
#define BIGDATA_IPROCESSOR_HH_

#include <cstdint>

#include <vector>
#include <memory>

namespace bigdata {

class AddressInfo;
class Server;

class IProcessor {
public:
    virtual ~IProcessor() = default;

    virtual void process(std::vector<uint8_t> buffer, std::shared_ptr<AddressInfo> addr, std::shared_ptr<Server> server) = 0;
};

}

#endif  // BIGDATA_IPROCESSOR_HH_
