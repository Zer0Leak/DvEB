

#ifndef BIGDATA_REGISTRY_HH_
#define BIGDATA_REGISTRY_HH_

#include <cstdint>

#include <unordered_map>
#include <mutex>

#include "bigdata/globals.hh"
#include "bigdata/natural.hh"
#include "bigdata/id.hh"
#include "bigdata/itree.hh"

namespace bigdata {

class Registry {
public:
    virtual ~Registry() {}
    virtual void sign(tree_uuid_t root_id, std::shared_ptr<ITree> iveb);
    virtual void sign(tree_uuid_t root_id, std::shared_ptr<const Id> id, std::shared_ptr<ITree> iveb);
    virtual void sign(tree_uuid_t root_id, const Id &id, std::shared_ptr<ITree> iveb);
    virtual void sign(tree_uuid_t root_id, Id && id, std::shared_ptr<ITree> iveb);

    virtual bool has(tree_uuid_t root_id, std::shared_ptr<const Id> id);
    virtual bool has(tree_uuid_t root_id, const Id &id);

    virtual std::shared_ptr<ITree> get(tree_uuid_t root_id, std::shared_ptr<const Id> id);
    virtual std::shared_ptr<ITree> get(tree_uuid_t root_id, const Id &id);

    virtual std::shared_ptr<ITree> get_root(tree_uuid_t root_id);
    virtual bool has_root(tree_uuid_t root_id);

    std::recursive_mutex & mutex() { return mutex_; }
protected:
    std::recursive_mutex mutex_;
    // [ root_id,  { root_ptr , [ tree_id , tree_ptr ] } ]
    std::unordered_map<tree_uuid_t,
        std::pair<std::shared_ptr<ITree>,
            std::unordered_map<Id, std::shared_ptr<ITree> > > > registry_;
};

}

#endif  // BIGDATA_REGISTRY_HH_
