/*
 * ProtoVEB.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef BIGDATA_ITREE_HH_
#define BIGDATA_ITREE_HH_

#include <utility>

#include "bigdata/natural.hh"
#include "bigdata/iserializable.hh"
#include "bigdata/id.hh"

namespace bigdata {

class ITree {
public:
    ITree() = default;
    ITree(const ITree &) = default;
    ITree(ITree &&) = default;
    virtual ~ITree() = default;


    virtual std::tuple<bool, std::shared_ptr<ISerializable>, bool> insert(const Natural & key,
            std::shared_ptr<ISerializable> value) = 0;
    virtual std::tuple<bool, std::shared_ptr<ISerializable>, bool> remove(const Natural & key) = 0;
    virtual std::pair<bool, std::shared_ptr<ISerializable>> search(const Natural & key) = 0;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> successor(const Natural & key) = 0;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> predecessor(const Natural & key) = 0;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min() const = 0;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max() const = 0;
    virtual bitscnt_t universe() = 0;
    virtual std::shared_ptr<const Id> id() = 0;

    virtual void touch() = 0;

    virtual bool exist() { return true; }
};

}

#endif  // BIGDATA_ITREE_HH_
