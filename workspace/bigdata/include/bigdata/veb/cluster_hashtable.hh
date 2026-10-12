#ifndef BIGDATA_VEB_CLUSTER_HASHTABLE_HH_
#define BIGDATA_VEB_CLUSTER_HASHTABLE_HH_

#include <cstdint>

#include <unordered_map>

#include "bigdata/veb/icluster.hh"
#include "bigdata/natural.hh"

namespace bigdata {
namespace veb {

class Veb;

class ClusterHashTable : public ICluster {
public:
    explicit ClusterHashTable(bitscnt_t bits);
    virtual ~ClusterHashTable() = default;

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
    std::unordered_map<Natural, std::shared_ptr<ITree>> hashtable_;
};

}
}

#endif  // BIGDATA_VEB_CLUSTER_HASHTABLE_HH_
