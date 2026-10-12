#include "bigdata/veb/veb_u1_summary_local.hh"
#include "bigdata/statistc.hh"

namespace bigdata {
namespace veb {

VebU1SummaryLocal::VebU1SummaryLocal(bitscnt_t universe, std::shared_ptr<const Id> id) : VebU1(universe, id) {
}


VebU1SummaryLocal::~VebU1SummaryLocal() {

}

std::pair<Natural, std::shared_ptr<ISerializable>> VebU1SummaryLocal::min() const {
    statistic::thread_sample_set_level(universe_);
    return std::make_pair(min_, nullptr);
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebU1SummaryLocal::max() const {
    statistic::thread_sample_set_level(universe_);
    return std::make_pair(max_, nullptr);
}


std::pair<bool, std::shared_ptr<ISerializable>> VebU1SummaryLocal::set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min) {
    statistic::thread_sample_set_level(universe_);
    Natural ret;
    std::swap(ret, min_);
    min_ = min.first;
    return std::make_pair(ret.is_valid(), nullptr);
}

std::pair<bool, std::shared_ptr<ISerializable>> VebU1SummaryLocal::set_max(std::pair<Natural, std::shared_ptr<ISerializable>> max) {
    statistic::thread_sample_set_level(universe_);
    Natural ret;
    std::swap(ret, max_);
    max_ = max.first;
    return std::make_pair(ret.is_valid(), nullptr);
}


}
}
