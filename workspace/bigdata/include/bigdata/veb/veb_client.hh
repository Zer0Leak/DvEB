/*
 * ProtoVEB.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef BIGDATA_VEB_CLIENT_HH_
#define BIGDATA_VEB_CLIENT_HH_

#include <utility>
#include <stdexcept>

#include "bigdata/veb/iveb_global.hh"

namespace bigdata {

class TreeRpc;

namespace veb {

class VebClient : public IVebGlobal {
public:
    VebClient(bitscnt_t universe, std::shared_ptr<const Id> id, std::shared_ptr<TreeRpc> tree_rpc);
    virtual ~VebClient() = default;
    virtual std::shared_ptr<const Id> id();

    virtual std::tuple<bool, std::shared_ptr<ISerializable>,bool> insert(const Natural & key,
            std::shared_ptr<ISerializable> value);
    virtual std::tuple<bool, std::shared_ptr<ISerializable>,bool> remove(const Natural & key);
    virtual std::pair<bool, std::shared_ptr<ISerializable>> search(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> successor(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> predecessor(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min() const;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max() const;
    virtual bitscnt_t universe();

    virtual void touch();

    virtual bool exist();

    virtual void expanded(std::shared_ptr<ITree>);

protected:


    bitscnt_t universe_;
    std::shared_ptr<const Id> id_;
    std::shared_ptr<TreeRpc> tree_rpc_;

    friend class VebRoot;
};

}
}

#endif  // BIGDATA_VEB_CLIENT_HH_
