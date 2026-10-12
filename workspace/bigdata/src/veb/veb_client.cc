
#include <boost/log/trivial.hpp>

#include "bigdata/veb/veb_client.hh"
#include "bigdata/tree_rpc_multicast.hh"
#include "bigdata/statistc.hh"
#include "bigdata/properties.hh"

namespace bigdata {

using namespace statistic;

namespace veb {

VebClient::VebClient(bitscnt_t universe, std::shared_ptr<const Id> id, std::shared_ptr<TreeRpc> tree_rpc)
    : universe_(universe), id_(id), tree_rpc_(tree_rpc)
{
}

std::shared_ptr<const Id> VebClient::id() {
    return id_;
}

std::tuple<bool, std::shared_ptr<ISerializable>,bool> VebClient::insert(const Natural & key, std::shared_ptr<ISerializable> value) {
    try {
        return tree_rpc_->insert(key, value);
    } catch (const exception_peer_not_exist &e) {
        tree_rpc_->create_node(universe_, id_);
        return tree_rpc_->insert(key, value);
    } catch (const exception_peer_timedout &e) {
        if (Properties::get_instance()->self_cheating() == true || Properties::get_instance()->has_cheater() == true) {
            throw e;
        }
        tree_rpc_->create_node(universe_, id_);
        return tree_rpc_->insert(key, value);
    }
}

std::tuple<bool, std::shared_ptr<ISerializable>,bool> VebClient::remove(const Natural & key) {
    try {
        return tree_rpc_->remove(key);
    } catch (const exception_peer_not_exist &e) {
        return {};
    } catch (const exception_peer_timedout &e) {
        if (Properties::get_instance()->self_cheating() == true || Properties::get_instance()->has_cheater() == true) {
            throw e;
        } else {
            return {};
        }
    }
}

std::pair<bool, std::shared_ptr<ISerializable>> VebClient::search(const Natural & key) {
    try {
        return tree_rpc_->search(key);
    } catch (const exception_peer_not_exist &e) {
        return {};
    } catch (const exception_peer_timedout &e) {
        if (Properties::get_instance()->self_cheating() == true || Properties::get_instance()->has_cheater() == true) {
            BOOST_LOG_TRIVIAL(error) << "search failed id " << *id_ << " : " << e.what();
            throw e;
        } else {
            return {};
        }
    }
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebClient::successor(const Natural & key) {
    try {
        return tree_rpc_->successor(key);
    } catch (const exception_peer_not_exist &e) {
        return {};
    } catch (const exception_peer_timedout &e) {
        if (Properties::get_instance()->self_cheating() == true || Properties::get_instance()->has_cheater() == true) {
            throw e;
        } else {
            return {};
        }
    }
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebClient::predecessor(const Natural & key) {
    try {
        return tree_rpc_->predecessor(key);
    } catch (const exception_peer_not_exist &e) {
        return {};
    } catch (const exception_peer_timedout &e) {
        if (Properties::get_instance()->self_cheating() == true || Properties::get_instance()->has_cheater() == true) {
            throw e;
        } else {
            return {};
        }
    }
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebClient::min() const {
    try {
        return tree_rpc_->min();
    } catch (const exception_peer_not_exist &e) {
        return {};
    } catch (const exception_peer_timedout &e) {
        if (Properties::get_instance()->self_cheating() == true || Properties::get_instance()->has_cheater() == true) {
            throw e;
        } else {
            return {};
        }
    }
}

std::pair<Natural, std::shared_ptr<ISerializable>> VebClient::max() const {
    try {
        return tree_rpc_->max();
    } catch (const exception_peer_not_exist & e) {
        return {};
    } catch (const exception_peer_timedout & e) {
        if (Properties::get_instance()->self_cheating() == true || Properties::get_instance()->has_cheater() == true) {
            throw e;
        } else {
            return {};
        }
    }
}

bool VebClient::exist() {
    try {
        tree_rpc_->min();
        return true;
    } catch (const exception_peer_not_exist &) {
        return false;
    } catch (const exception_peer_timedout &e) {
        if (Properties::get_instance()->self_cheating() == true || Properties::get_instance()->has_cheater() == true) {
            throw e;
        } else {
            return false;
        }
    }
}

bitscnt_t VebClient::universe() {
    return universe_;
    // return tree_rpc_->universe();
}

void VebClient::touch() {
    try {
        tree_rpc_->min();
    } catch (const exception_peer_not_exist &) {
        tree_rpc_->create_node(universe_, id_);
    } catch (const exception_peer_timedout & e) {
        if (Properties::get_instance()->self_cheating() == true || Properties::get_instance()->has_cheater() == true) {
            throw e;
        } else {
            tree_rpc_->create_node(universe_, id_);
        }
    }
}

void VebClient::expanded(std::shared_ptr<ITree> tree) {
    try {
        tree_rpc_->veb_expanded(tree);
    } catch (const exception_peer_not_exist &) {
        tree_rpc_->create_node(universe_, id_);
        tree_rpc_->veb_expanded(tree);
    } catch (const exception_peer_timedout &e) {
        if (Properties::get_instance()->self_cheating() == true || Properties::get_instance()->has_cheater() == true) {
            throw e;
        } else {
            tree_rpc_->create_node(universe_, id_);
            tree_rpc_->veb_expanded(tree);
        }
    }
}

}
}
