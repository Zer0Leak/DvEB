#include <stdexcept>

#include <boost/log/trivial.hpp>

#include "bigdata/registry.hh"
#include "bigdata/util.hh"

namespace bigdata {

void Registry::sign(tree_uuid_t root_id, std::shared_ptr<ITree> iveb) {
    std::lock_guard<std::recursive_mutex>lg(mutex_);
    auto itroot = registry_.find(root_id);

    if (UNLIKELY(itroot == registry_.end())) {
        // [ root_id,  { root_ptr , [ tree_id , tree_ptr ] } ]

        registry_.insert({root_id, {iveb, {}}});

        if (iveb == NULL) {
            BOOST_LOG_TRIVIAL(info) << "Hosting root " << root_id;
        } else {
            BOOST_LOG_TRIVIAL(info) << "Owning root " << root_id;
        }
    }
}

void Registry::sign(tree_uuid_t root_id, const Id &id, std::shared_ptr<ITree> iveb) {
    std::lock_guard<std::recursive_mutex>lg(mutex_);
    Registry::sign(root_id, Id(id), std::shared_ptr<ITree>(iveb));  // call move version with new rvalue
}

void Registry::sign(tree_uuid_t root_id, std::shared_ptr<const Id> id, std::shared_ptr<ITree> iveb) {
    std::lock_guard<std::recursive_mutex>lg(mutex_);
    if (id != NULL) {
        Registry::sign(root_id, *id, iveb);
    }
}

void Registry::sign(tree_uuid_t root_id, Id && id, std::shared_ptr<ITree> iveb) {
    std::lock_guard<std::recursive_mutex>lg(mutex_);
    auto itroot = registry_.find(root_id);

    if (UNLIKELY(itroot == registry_.end())) {
        registry_.insert({root_id, {{}, {}}});
        itroot = registry_.find(root_id);
    }

    auto itree = itroot->second.second.find(id);

    if (UNLIKELY(itree != itroot->second.second.end())) {
        throw std::runtime_error("Tree already resgistered.");
    }

    itroot->second.second.insert({id, iveb});
}

std::shared_ptr<ITree> Registry::get(tree_uuid_t root_id, std::shared_ptr<const Id> id) {
    std::lock_guard<std::recursive_mutex>lg(mutex_);
    if (id != NULL) {
        return Registry::get(root_id, *id);
    } else {
        return {};
    }
}

bool Registry::has(tree_uuid_t root_id, std::shared_ptr<const Id> id) {
    std::lock_guard<std::recursive_mutex>lg(mutex_);
    if (id != NULL) {
        return Registry::has(root_id, *id);
    } else {
        return false;
    }
}

bool Registry::has(tree_uuid_t root_id, const Id &id) {
    std::lock_guard<std::recursive_mutex>lg(mutex_);
    auto itroot = registry_.find(root_id);

    if (itroot == registry_.end()) {
        return false;
    }

    auto itree = itroot->second.second.find(id);

    if (UNLIKELY(itree == itroot->second.second.end())) {
        return false;
    }

    return true;
}

std::shared_ptr<ITree> Registry::get(tree_uuid_t root_id, const Id &id) {
    std::lock_guard<std::recursive_mutex>lg(mutex_);
    auto itroot = registry_.find(root_id);

    if (itroot == registry_.end()) {
        return {};
    }

    auto itree = itroot->second.second.find(id);

    if (UNLIKELY(itree == itroot->second.second.end())) {
        return {};
    }

    return itree->second;
}

std::shared_ptr<ITree> Registry::get_root(tree_uuid_t root_id) {
    std::lock_guard<std::recursive_mutex>lg(mutex_);
    auto itroot = registry_.find(root_id);

    if (itroot != registry_.end()) {
        return itroot->second.first;
    }

    return {};
}

bool Registry::has_root(tree_uuid_t root_id) {
    std::lock_guard<std::recursive_mutex>lg(mutex_);
    auto itroot = registry_.find(root_id);

    if (itroot != registry_.end()) {
        return true;
    }

    return false;
}

}
