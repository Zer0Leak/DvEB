#ifndef BIGDATA_VEB_U1_SUMMARY_LOCAL_HH_
#define BIGDATA_VEB_U1_SUMMARY_LOCAL_HH_

#include "bigdata/veb/veb_u1.hh"

namespace bigdata {
namespace veb {

class VebU1SummaryLocal : public VebU1 {
public:
    VebU1SummaryLocal(bitscnt_t universe, std::shared_ptr<const Id> id);
    virtual ~VebU1SummaryLocal();

    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min() const;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max() const;

protected:
    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min);
    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_max(std::pair<Natural, std::shared_ptr<ISerializable>> max);

    Natural min_;
    Natural max_;
};

}
}

#endif  // BIGDATA_VEB_U1_SUMMARY_LOCAL_HH_
