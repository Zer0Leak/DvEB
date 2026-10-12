
#include "bigdata/veb/veb_local.hh"
#include "bigdata/veb/veb_summary_local.hh"
#include "bigdata/veb/veb_u1_summary_local.hh"
#include "bigdata/veb/cluster_array.hh"
#include "bigdata/veb/cluster_hashtable.hh"
#include "bigdata/statistc.hh"

namespace bigdata {
namespace veb {

VebLocal::VebLocal(bitscnt_t universe, std::shared_ptr<const Id> id) : Veb(universe, id) {
    constructor_helper();
}

void VebLocal::constructor_helper() {
    if (universe_ == 2) {
        summary_ = std::make_shared<VebU1SummaryLocal>(universe_ >> 1, nullptr);
    } else {
        summary_ = std::make_shared<VebSummaryLocal>(universe_ >> 1, nullptr);
    }

    if (universe_ <= 32) {
        cluster_ = std::make_shared<ClusterArray>(universe_ >> 1);
    } else {
        cluster_ = std::make_shared<ClusterHashTable>(universe_ >> 1);
    }
}

VebLocal::~VebLocal () {

}

std::pair<Natural, std::shared_ptr<ISerializable>> VebLocal::min() const {
    statistic::thread_sample_set_level(universe_);
    return min_;
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebLocal::max() const {
    statistic::thread_sample_set_level(universe_);
    return max_;
}

void VebLocal::expanded(std::shared_ptr<ITree> tree) {
    statistic::thread_sample_set_level(universe_);
    if (tree == NULL) return;

    auto min = tree->min();

    if (min.first.is_valid()) {
        // remove min from child
        tree->remove(min.first);

        // if child is not empty update summary
        auto child_min = tree->min();
        if (child_min.first.is_valid()) {
            Natural high = child_min.first.high(universe_ >> 1);
            summary_->insert(high, {});
        }

        // set max to chield's max
        max_ = tree->max();
        // set min to child's min
        swap(min_, min);
    }

    // Update the cluster at 0
    // even if there is nothing, lets not allow it to be unallocated
    cluster_->expand_set(tree);
}

std::pair<bool, std::shared_ptr<ISerializable>> VebLocal::set_min(std::pair<Natural, std::shared_ptr<ISerializable>> min) {
    statistic::thread_sample_set_level(universe_);
    std::pair<Natural, std::shared_ptr<ISerializable>> ret;
    std::swap(ret, min_);
    min_ = min;
    return {ret.first.is_valid(), ret.second};
}

std::pair<bool, std::shared_ptr<ISerializable>> VebLocal::set_max(std::pair<Natural, std::shared_ptr<ISerializable>> max) {
    statistic::thread_sample_set_level(universe_);
    std::pair<Natural, std::shared_ptr<ISerializable>> ret;
    std::swap(ret, max_);
    max_ = max;
    return {ret.first.is_valid(), ret.second};
}

std::shared_ptr<ITree> VebLocal::summary() const {
    return  summary_;
}

std::shared_ptr<ICluster> VebLocal::cluster() const {
    return cluster_;
}

}
}
