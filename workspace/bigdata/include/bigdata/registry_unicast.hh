

#ifndef BIGDATA_REGISTRY_UNICAST_HH_
#define BIGDATA_REGISTRY_UNICAST_HH_

#include "bigdata/registry.hh"

namespace bigdata {

class RegistryUnicast : public Registry {
public:
    RegistryUnicast() = default;
    virtual ~RegistryUnicast()  = default;
};

}

#endif  // BIGDATA_REGISTRY_UNICAST_HH_
