#ifndef BIGDATA_VEB_ROOT_HH_
#define BIGDATA_VEB_ROOT_HH_

#include <memory>

#include "bigdata/veb/iveb_global.hh"
#include "bigdata/globals.hh"

namespace bigdata {

class Factory;
class Registry;
class Server;

namespace veb {

class VebRoot : public IVebGlobal {
public:
    VebRoot(tree_uuid_t uuid, std::shared_ptr<Server> server, std::shared_ptr<Factory> factory, std::shared_ptr<Registry> registry);
    virtual ~VebRoot();

    virtual std::tuple<bool, std::shared_ptr<ISerializable>,bool> insert(const Natural & key,
            std::shared_ptr<ISerializable> value);
    virtual std::tuple<bool, std::shared_ptr<ISerializable>,bool> remove(const Natural & key);
    virtual std::pair<bool, std::shared_ptr<ISerializable>> search(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> successor(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> predecessor(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min() const;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max() const;
    virtual bitscnt_t universe();
    virtual std::shared_ptr<const Id> id();
    tree_uuid_t uuid() const { return uuid_; }

    virtual void touch() {}

protected:
    virtual void expanded(std::shared_ptr<ITree> tree);

private:
    void expand(const Natural & key);

    std::shared_ptr<Server> server_;
    std::shared_ptr<Factory> factory_;
    std::shared_ptr<Registry> registry_;
    std::shared_ptr<IVebGlobal> veb_;
    tree_uuid_t uuid_;
    bitscnt_t universe_;
    std::shared_ptr<Id> id_;
};

}
}

#endif  // BIGDATA_VEB_ROOT_HH_
