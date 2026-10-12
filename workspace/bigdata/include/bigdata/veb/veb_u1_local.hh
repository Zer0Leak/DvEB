#ifndef BIGDATA_VEB_U1_LOCAL_HH_
#define BIGDATA_VEB_U1_LOCAL_HH_

#include "bigdata/veb/veb_u1.hh"

namespace bigdata {
namespace veb {

class VebU1Local : public VebU1 {
public:
    VebU1Local(bitscnt_t universe, std::shared_ptr<const Id> id);
    virtual ~VebU1Local();

    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min() const;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max() const;

protected:
    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min);
    virtual std::pair<bool, std::shared_ptr<ISerializable>> set_max(std::pair<Natural, std::shared_ptr<ISerializable>> max);

    std::pair<Natural, std::shared_ptr<ISerializable>> min_;
    std::pair<Natural, std::shared_ptr<ISerializable>> max_;
};

}
}

#endif  // BIGDATA_VEB_U1_LOCAL_HH_
