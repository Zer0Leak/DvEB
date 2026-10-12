#include "bigdata/veb/veb_summary_local.hh"
#include "bigdata/veb/veb_u1_summary_local.hh"
#include "bigdata/veb/cluster_summary_array.hh"
#include "bigdata/veb/cluster_summary_hashtable.hh"
#include "bigdata/statistc.hh"

namespace bigdata {
namespace veb {

VebSummaryLocal::VebSummaryLocal(bitscnt_t universe, std::shared_ptr<const Id> id) : Veb(universe, id)  {
    constructor_helper();
}

void VebSummaryLocal::constructor_helper() {
    if (universe_ == 2) {
        summary_ = std::make_shared<VebU1SummaryLocal>(universe_ >> 1, nullptr);
    } else {
        summary_ = std::make_shared<VebSummaryLocal>(universe_ >> 1, nullptr);
    }

    if (universe_ <= 32) {
        cluster_ = std::make_shared<ClusterSummaryArray>(universe_ >> 1);
    } else {
        cluster_ = std::make_shared<ClusterSummaryHashTable>(universe_ >> 1);
    }
}

VebSummaryLocal::~VebSummaryLocal() {

}

std::pair<Natural, std::shared_ptr<ISerializable>> VebSummaryLocal::min() const {
    statistic::thread_sample_set_level(universe_);
    return {min_, {}};
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebSummaryLocal::max() const {
    statistic::thread_sample_set_level(universe_);
    return {max_, {}};
}


void VebSummaryLocal::expanded(std::shared_ptr<ITree> tree) {
    throw std::runtime_error("Can't use force methods on the Root.");
}

std::pair<bool, std::shared_ptr<ISerializable>> VebSummaryLocal::set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min) {
    statistic::thread_sample_set_level(universe_);
    Natural ret;
    std::swap(ret, min_);
    min_ = min.first;
    return {ret.is_valid(), {}};
}

std::pair<bool, std::shared_ptr<ISerializable>> VebSummaryLocal::set_max(std::pair<Natural, std::shared_ptr<ISerializable>> max) {
    statistic::thread_sample_set_level(universe_);
    Natural ret;
    std::swap(ret, max_);
    max_ = max.first;
    return {ret.is_valid(), {}};
}

std::shared_ptr<ITree> VebSummaryLocal::summary() const {
    return summary_;
}

std::shared_ptr<ICluster> VebSummaryLocal::cluster() const {
    return cluster_;
}

}
}
