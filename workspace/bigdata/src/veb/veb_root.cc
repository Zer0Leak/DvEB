#include <memory>
#include <exception>

#include "bigdata/veb/veb_root.hh"
#include "bigdata/util.hh"
#include "bigdata/factory.hh"
#include "bigdata/registry.hh"
#include "bigdata/server.hh"
#include "bigdata/statistc.hh"
#include "bigdata/properties.hh"

namespace bigdata {
namespace veb {


VebRoot::VebRoot(tree_uuid_t uuid, std::shared_ptr<Server> server, std::shared_ptr<Factory> factory, std::shared_ptr<Registry> registry)
    : server_(server)
    , factory_(factory)
    , registry_(registry)
    , uuid_(uuid)
    , universe_(0)
{
}

VebRoot::~VebRoot() {

}

void VebRoot::expand(const Natural & key) {
    while (key.bits() > universe_) {
        auto start = std::chrono::steady_clock::now();
        auto sample = statistic::include_thread_sample();
        auto aux_universe = universe_;
        auto time_couter = std::shared_ptr<void>(NULL, [&start,&sample, aux_universe](void*){
            if (sample != NULL && false == std::uncaught_exception()) {
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

                sample->normalize_level(aux_universe);
                sample->duration = duration.count();
                statistic::Statistic::add(*sample, TreeRpc::kVeb_expanded);
            }
        });


        bitscnt_t new_universe = (universe_ > 0) ? universe_ << 1 : 1;
        std::shared_ptr<IVebGlobal> new_veb;
        auto temp_id = std::make_shared<Id>(Id::calculate_id(new_universe));
        auto newtuple = factory_->create(uuid_, new_universe, temp_id, server_, Factory::kTreeAuto, Factory::kInstanceClient);

        new_veb = std::dynamic_pointer_cast<IVebGlobal>(std::get<2>(newtuple));
        new_veb->touch();

        if (veb_ != NULL) {
            new_veb->expanded(veb_);
        }

        if (std::get<1>(newtuple) == Factory::kInstanceLocal) {
            registry_->sign(uuid_ , *new_veb->id(), new_veb);
        }

        veb_.swap(new_veb);
        universe_ = new_universe;
        std::swap(id_, temp_id);
    }
}

std::tuple<bool, std::shared_ptr<ISerializable>,bool> VebRoot::insert(const Natural & key,
                                                                   std::shared_ptr<ISerializable> value) {
    if (!key.is_valid()) return {};
    if (veb_ == NULL || key.bits() > universe_) {
        expand(key);
    }

    auto start = std::chrono::steady_clock::now();
    auto sample = statistic::include_thread_sample();
    auto time_couter = std::shared_ptr<void>(NULL, [this,&start,&sample](void*){
        if (sample != NULL && false == std::uncaught_exception()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

            sample->normalize_level(universe_);
            sample->duration = duration.count();
            statistic::Statistic::add(*sample, TreeRpc::kInsert);
        }
    });

    return veb_->insert(key, value);
}

std::tuple<bool, std::shared_ptr<ISerializable>,bool> VebRoot::remove(const Natural & key) {
    if (UNLIKELY(veb_ == NULL)) {
        return {};
    }

    auto start = std::chrono::steady_clock::now();
    auto sample = statistic::include_thread_sample();
    auto time_couter = std::shared_ptr<void>(NULL, [this,&start,&sample](void*){
        if (sample != NULL && false == std::uncaught_exception()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

            sample->normalize_level(universe_);
            sample->duration = duration.count();
            statistic::Statistic::add(*sample, TreeRpc::kRemove);
        }
    });

    return veb_->remove(key);
}

std::pair<bool, std::shared_ptr<ISerializable>> VebRoot::search(const Natural & key) {
    if (UNLIKELY(veb_ == NULL)) {
        return {};
    }

    auto start = std::chrono::steady_clock::now();
    auto sample = statistic::include_thread_sample();
    auto time_couter = std::shared_ptr<void>(NULL, [this,&start,&sample](void*){
        if (sample != NULL && false == std::uncaught_exception()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

            sample->normalize_level(universe_);
            sample->duration = duration.count();
            statistic::Statistic::add(*sample, TreeRpc::kSearch);
        }
    });

    return veb_->search(key);
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebRoot::successor(const Natural & key) {
    if (UNLIKELY(veb_ == NULL)) {
        return {};
    }

    auto start = std::chrono::steady_clock::now();
    auto sample = statistic::include_thread_sample();
    auto time_couter = std::shared_ptr<void>(NULL, [this,&start,&sample](void*){
        if (sample != NULL && false == std::uncaught_exception()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

            sample->normalize_level(universe_);
            sample->duration = duration.count();
            statistic::Statistic::add(*sample, TreeRpc::kSuccessor);
        }
    });

    return veb_->successor(key);
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebRoot::predecessor(const Natural & key) {
    if (UNLIKELY(veb_ == NULL)) {
        return {};
    }

    auto start = std::chrono::steady_clock::now();
    auto sample = statistic::include_thread_sample();
    auto time_couter = std::shared_ptr<void>(NULL, [this,&start,&sample](void*){
        if (sample != NULL && false == std::uncaught_exception()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

            sample->normalize_level(universe_);
            sample->duration = duration.count();
            statistic::Statistic::add(*sample, TreeRpc::kPredecessor);
        }
    });

    return veb_->predecessor(key);
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebRoot::min() const {
    if (UNLIKELY(veb_ == NULL)) {
        return {};
    }

    auto start = std::chrono::steady_clock::now();
    auto sample = statistic::include_thread_sample();
    auto time_couter = std::shared_ptr<void>(NULL, [this,&start,&sample](void*){
        if (sample != NULL && false == std::uncaught_exception()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

            sample->normalize_level(universe_);
            sample->duration = duration.count();
            statistic::Statistic::add(*sample, TreeRpc::kMin);
        }
    });

    return veb_->min();
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebRoot::max() const {
    if (UNLIKELY(veb_ == NULL)) {
        return {};
    }

    auto start = std::chrono::steady_clock::now();
    auto sample = statistic::include_thread_sample();
    auto time_couter = std::shared_ptr<void>(NULL, [this,&start,&sample](void*){
        if (sample != NULL && false == std::uncaught_exception()) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

            sample->normalize_level(universe_);
            sample->duration = duration.count();
            statistic::Statistic::add(*sample, TreeRpc::kMax);
        }
    });

    return veb_->max();
}

bitscnt_t VebRoot::universe() {
    return universe_;
}

std::shared_ptr<const Id> VebRoot::id() {
    if (UNLIKELY(veb_ == NULL)) {
        throw std::runtime_error("There is no underying Veb.");
    }

    return id_;
}

void VebRoot::expanded(std::shared_ptr<ITree>) {
    throw std::runtime_error("Can't use force methods on the Root.");
}

}
}
