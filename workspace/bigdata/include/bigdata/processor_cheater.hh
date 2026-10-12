
#ifndef BIGDATA_PROCESSOR_CHEATER_HH_
#define BIGDATA_PROCESSOR_CHEATER_HH_

#include "bigdata/iprocessor.hh"

namespace bigdata {

class Registry;
class Server;
class Factory;

class ProcessorCheater : public IProcessor {
public:
    ProcessorCheater(std::shared_ptr<Registry> registry, std::shared_ptr<Factory> factory);
    virtual ~ProcessorCheater() = default;

    virtual void process(std::vector<uint8_t> buffer, std::shared_ptr<AddressInfo> addr, std::shared_ptr<Server> server);

private:
    std::shared_ptr<Registry> registry_;
    std::shared_ptr<Factory> factory_;
};

}

#endif  // BIGDATA_PROCESSOR_CHEATER_HH_
