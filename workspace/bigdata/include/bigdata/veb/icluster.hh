#ifndef BIGDATA_VEB_ICLUSTER_HH_
#define BIGDATA_VEB_ICLUSTER_HH_

#include <memory>

#include "bigdata/natural.hh"
#include "bigdata/iserializable.hh"

namespace bigdata {

class ITree;

namespace veb {

class ICluster {
public:
    virtual ~ICluster() = default;

    virtual std::shared_ptr<ITree> at(const Natural & index) const = 0;
    virtual std::shared_ptr<ITree> alloc(const Natural & index) = 0;

    virtual std::tuple<bool, std::shared_ptr<ISerializable>, bool> insert(const Natural & index, const Natural & key, std::shared_ptr<ISerializable> value) = 0;
    virtual std::tuple<bool, std::shared_ptr<ISerializable>, bool> remove(const Natural & index, const Natural & key) = 0;
    virtual std::pair<bool, std::shared_ptr<ISerializable>> search(const Natural & index, const Natural & key) = 0;

    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min(const Natural & index) const = 0;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max(const Natural & index) const = 0;

    virtual void expand_set(std::shared_ptr<ITree>) = 0;
};

}
}

#endif  // BIGDATA_VEB_CLUSTER_HH_
