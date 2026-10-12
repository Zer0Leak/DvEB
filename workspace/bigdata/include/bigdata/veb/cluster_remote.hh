#ifndef BIGDATA_VEB_CLUSTER_REMOTE_HH_
#define BIGDATA_VEB_CLUSTER_REMOTE_HH_

#include <cstdint>

#include "bigdata/veb/icluster.hh"
#include "bigdata/globals.hh"

namespace bigdata {

class Id;
class Server;
class Factory;

namespace veb {

class Veb;

class ClusterRemote : public ICluster {
public:
    explicit ClusterRemote(bitscnt_t bits, std::shared_ptr<const Id> tree_id, std::shared_ptr<Server> server, std::shared_ptr<Factory> factory, tree_uuid_t uuid);
    virtual ~ClusterRemote() = default;

    virtual std::shared_ptr<ITree> at(const Natural & index) const;
    virtual std::shared_ptr<ITree> alloc(const Natural & index);

    virtual std::tuple<bool, std::shared_ptr<ISerializable>, bool> insert(const Natural & index, const Natural & key, std::shared_ptr<ISerializable> value);
    virtual std::tuple<bool, std::shared_ptr<ISerializable>, bool> remove(const Natural & index, const Natural & key);
    virtual std::pair<bool, std::shared_ptr<ISerializable>> search(const Natural & index, const Natural & key);

    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min(const Natural & index) const;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max(const Natural & index) const;

    virtual void expand_set(std::shared_ptr<ITree>);

private:
    bitscnt_t bits_;
    std::shared_ptr<const Id> tree_id_;
    std::shared_ptr<Server> server_;
    std::shared_ptr<Factory> factory_;
    tree_uuid_t uuid_;
};

}
}

#endif  // BIGDATA_VEB_CLUSTER_REMOTE_HH_
