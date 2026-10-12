#ifndef BIGDATA_VEB_REMOTE_HH_
#define BIGDATA_VEB_REMOTE_HH_

#include "bigdata/veb/veb.hh"
#include "bigdata/globals.hh"

namespace bigdata {

class Server;
class Factory;

namespace veb {

class VebRemote : public Veb {
public:
    VebRemote(bitscnt_t universe, std::shared_ptr<const Id> id, tree_uuid_t uuid, std::shared_ptr<Server> server, std::shared_ptr<Factory> factory);
    virtual ~VebRemote    ();

    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min() const;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max() const;

protected:
    virtual void expanded(std::shared_ptr<ITree> tree);

    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min);
    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_max(std::pair<Natural, std::shared_ptr<ISerializable>> min);

    virtual std::shared_ptr<ITree> summary() const;
    virtual std::shared_ptr<ICluster> cluster() const;

    std::shared_ptr<ITree> summary_;
    std::shared_ptr<ICluster> cluster_;

    std::pair<Natural, std::shared_ptr<ISerializable>> min_;
    std::pair<Natural, std::shared_ptr<ISerializable>> max_;
private:
    void constructor_helper();
    tree_uuid_t uuid_;
    std::shared_ptr<Server> server_;
    std::shared_ptr<Factory> factory_;
};

}
}

#endif  // BIGDATA_VEB_REMOTE_HH_
