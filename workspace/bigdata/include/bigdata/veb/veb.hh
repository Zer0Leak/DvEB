#ifndef BIGDATA_VEB_HH_
#define BIGDATA_VEB_HH_

#include <memory>

#include "bigdata/veb/iveb_global.hh"

namespace bigdata {

class Factory;
class Registry;

namespace veb {

class ICluster;

class Veb : public IVebGlobal {
public:
    Veb(bitscnt_t universe, std::shared_ptr<const Id> id);
    virtual ~Veb    ();

    virtual std::tuple<bool, std::shared_ptr<ISerializable>, bool> insert(const Natural & key,
            std::shared_ptr<ISerializable> value);
    virtual std::tuple<bool, std::shared_ptr<ISerializable>, bool> remove(const Natural & key);
    virtual std::pair<bool, std::shared_ptr<ISerializable>> search(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> successor(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> predecessor(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min() const = 0;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max() const = 0;
    virtual bitscnt_t universe();
    virtual std::shared_ptr<const Id> id();

    virtual void touch() {}

protected:
    virtual std::shared_ptr<ITree> summary() const = 0;
    virtual std::shared_ptr<ICluster> cluster() const = 0;

    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min) = 0;
    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_max(std::pair<Natural, std::shared_ptr<ISerializable>> max) = 0;

    std::shared_ptr<Factory> factory_;
    std::shared_ptr<Registry> registry_;
    std::shared_ptr<IVebGlobal> veb_;

    bitscnt_t universe_;
    std::shared_ptr<const Id> id_;
};

}
}

#endif  // BIGDATA_VEB_HH_
