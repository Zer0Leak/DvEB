#ifndef BIGDATA_VEB_U1_HH_
#define BIGDATA_VEB_U1_HH_

#include "bigdata/veb/veb.hh"
#include "bigdata/veb/iveb_global.hh"

namespace bigdata {
namespace veb {

class ICluster;

class VebU1 : public IVebGlobal {
public:
    VebU1(bitscnt_t universe, std::shared_ptr<const Id> id);
    virtual ~VebU1();

    virtual std::tuple<bool, std::shared_ptr<ISerializable>, bool> insert(const Natural & key,
            std::shared_ptr<ISerializable> value);
    virtual std::tuple<bool, std::shared_ptr<ISerializable>, bool> remove(const Natural & key);
    virtual std::pair<bool, std::shared_ptr<ISerializable>> search(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> successor(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> predecessor(const Natural & key);

    virtual bitscnt_t universe();
    virtual std::shared_ptr<const Id> id();

    virtual void touch() {}

protected:
    virtual void expanded(std::shared_ptr<ITree> tree);

    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min) = 0;
    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_max(std::pair<Natural, std::shared_ptr<ISerializable>> max) = 0;

    virtual std::shared_ptr<ITree> summary() const;
    virtual std::shared_ptr<ICluster> cluster() const;

    bitscnt_t universe_;
    std::shared_ptr<const Id> id_;
};

}
}

#endif  // BIGDATA_VEB_U1_HH_
