#ifndef BIGDATA_VEB_SUMMARY_LOCAL_HH_
#define BIGDATA_VEB_SUMMARY_LOCAL_HH_

#include "bigdata/veb/veb.hh"

namespace bigdata {
namespace veb {

class ClusterSummaryArray;

class VebSummaryLocal : public Veb {
public:
    VebSummaryLocal(bitscnt_t universe, std::shared_ptr<const Id> id);
    virtual ~VebSummaryLocal    ();

    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min() const;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max() const;

protected:
    virtual void expanded(std::shared_ptr<ITree> tree);

    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min);
    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_max(std::pair<Natural, std::shared_ptr<ISerializable>> max);

    virtual std::shared_ptr<ITree> summary() const;
    virtual std::shared_ptr<ICluster> cluster() const;

    std::shared_ptr<IVebGlobal> summary_;
    std::shared_ptr<ICluster> cluster_;

    Natural min_;
    Natural max_;
private:
    void constructor_helper();
};

}
}

#endif  // BIGDATA_VEB_SUMMARY_LOCAL_HH_
