
#include <error.h>

#include <cstring>
#include <cstdio>

#include <string>
#include <stdexcept>
#include <condition_variable>

#include <boost/log/trivial.hpp>

#include "bigdata/tree_rpc_multicast.hh"
#include "bigdata/channel_multicast.hh"
#include "bigdata/util.hh"
#include "bigdata/properties.hh"
#include "bigdata/statistc.hh"
#include "bigdata/flow_control.hh"

namespace bigdata {


exception_peer_not_exist::exception_peer_not_exist()
    : std::runtime_error("peer doesn't exist") {

}

exception_peer_not_exist::exception_peer_not_exist(const std::string & what_arg)
    : std::runtime_error(what_arg)
{
}

exception_peer_not_exist::exception_peer_not_exist(const char* what_arg)
    : std::runtime_error(what_arg)
{
}

exception_peer_timedout::exception_peer_timedout()
    : std::runtime_error("peer timeout")
{
}

exception_peer_timedout::exception_peer_timedout(const std::string & what_arg)
    : std::runtime_error(what_arg)
{
}

exception_peer_timedout::exception_peer_timedout(const char* what_arg)
    : std::runtime_error(what_arg)
{
}

TreeRpcMulticast::TreeRpcMulticast(bitscnt_t universe, std::shared_ptr<Server> server, tree_uuid_t uuid, std::shared_ptr<const Id> id, bool to_root)
    : server_(server)
    , uuid_(uuid)
    , id_(id)
    , to_root_(to_root)
    , universe_(universe)
{
}

size_t TreeRpcMulticast::process_header(Method method, tree_uuid_t uuid, const Transaction & transaction, bitscnt_t universe) const {
    size_t nbytes = 0;
    size_t ret = 0;

    ret = fwrite(&method, sizeof(method), 1, *(server_->channel()));
    if (ret != 1) {
        throw std::runtime_error("Can't write method. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(method);

    u_int16_t port = Properties::get_instance()->port();
    ret = fwrite(&port, sizeof(port), 1, *(server_->channel()));
    if (ret != 1) {
        throw std::runtime_error("Can't write port. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(port);

    ret = fwrite(&uuid, sizeof(uuid), 1, *(server_->channel()));
    if (ret != 1) {
        throw std::runtime_error("Can't write uuid. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(uuid);

    ret = transaction.write(*(server_->channel()));
    if (ret <= 0) {
        throw std::runtime_error("Can't write transaction.");
    }
    nbytes += ret;

    ret = fwrite(&universe, sizeof(universe), 1, *(server_->channel()));
    if (ret != 1) {
        throw std::runtime_error("Can't write universe. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(universe);

    return nbytes;
}

size_t TreeRpcMulticast::process_reply(Method method, const Transaction & transaction, tree_uuid_t uuid, bitscnt_t universe, bool has, bool had_something, const Natural * key, const ISerializable * value, const statistic::Sample * sample, const Id * id) const {
    size_t nbytes = 0;
    size_t ret = 0;

    nbytes = process_header(method, uuid, transaction, universe);

    ret = fwrite(&has, sizeof(has), 1, *(server_->channel()));
    if (ret != 1) {
        throw std::runtime_error("Can't write has. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(has);

    ret = fwrite(&had_something, sizeof(had_something), 1, *(server_->channel()));
    if (ret != 1) {
        throw std::runtime_error("Can't write had_something. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(had_something);

    if (key != NULL) {
        bool has_key = true;
        ret = fwrite(&has_key, sizeof(has_key), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_key. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_key);

        ret = key->write(*(server_->channel()));
        if (ret == 0) {
            throw std::runtime_error("Can't write Key. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    } else {
        bool has_key = false;
        ret = fwrite(&has_key, sizeof(has_key), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_key. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_key);
    }

    if (value != NULL) {
        bool has_value = true;
        ret = fwrite(&has_value, sizeof(has_value), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_value. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_value);

        ret = value->write(*(server_->channel()));
        if (ret <= 0) {
            throw std::runtime_error("Can't write Value. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    } else {
        bool has_value = false;
        ret = fwrite(&has_value, sizeof(has_value), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_value. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_value);
    }

    if (sample != NULL) {
        bool has_sample = true;
        ret = fwrite(&has_sample, sizeof(has_sample), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_sample. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_sample);

        ret = sample->write(*(server_->channel()));
        if (ret == 0) {
            throw std::runtime_error("Can't write sample. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    } else {
        bool has_sample = false;
        ret = fwrite(&has_sample, sizeof(has_sample), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_sample. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_sample);
    }

    if (id != NULL) {
        bool has_id = true;
        ret = fwrite(&has_id, sizeof(has_id), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_id. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_id);

        ret = id->write(*(server_->channel()));
        if (ret == 0) {
            throw std::runtime_error("Can't write Id. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    } else {
        bool has_id = false;
        ret = fwrite(&has_id, sizeof(has_id), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_Id. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_id);
    }

    return nbytes;
}

size_t TreeRpcMulticast::process(Method method, bool new_trans, const Natural * key, const ISerializable * value, const Id * expand_id, const statistic::Sample * sample) const {
    size_t nbytes = 0;
    size_t ret = 0;

    if (new_trans) {
        transaction_.set_mac((server_->channel())->mac_address());
        transaction_.renew();
    }

    nbytes = process_header(method, uuid_, transaction_, universe_);

    ret = fwrite(&to_root_, sizeof(to_root_), 1, *(server_->channel()));
    if (ret != 1) {
        throw std::runtime_error("Can't write to_root_. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(to_root_);

    if (!to_root_) {
        ret = id_->write(*(server_->channel()));
        if (ret <= 0) {
            throw std::runtime_error("Can't write Id. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    }

    if (key != NULL) {
        bool has_key = true;
        ret = fwrite(&has_key, sizeof(has_key), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_key. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_key);

        ret = key->write(*(server_->channel()));
        if (ret <= 0) {
            throw std::runtime_error("Can't write Key. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    } else {
        bool has_key = false;
        ret = fwrite(&has_key, sizeof(has_key), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_key. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_key);
    }

    if (value != NULL) {
        bool has_value = true;
        ret = fwrite(&has_value, sizeof(has_value), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_value. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_value);

        ret = value->write(*(server_->channel()));
        if (ret <= 0) {
            throw std::runtime_error("Can't write Value. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    } else {
        bool has_value = false;
        ret = fwrite(&has_value, sizeof(has_value), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_value. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_value);
    }

    if (expand_id != NULL) {
        bool has_expand_id = true;
        ret = fwrite(&has_expand_id, sizeof(has_expand_id), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_expand_id. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_expand_id);

        ret = expand_id->write(*(server_->channel()));
        if (ret <= 0) {
            throw std::runtime_error("Can't write expand_id. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    } else {
        bool has_expand_id = false;
        ret = fwrite(&has_expand_id, sizeof(has_expand_id), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_expand_id. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_expand_id);
    }

    if (sample != NULL) {
        bool has_sample = true;
        ret = fwrite(&has_sample, sizeof(has_sample), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_sample. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_sample);

        ret = sample->write(*(server_->channel()));
        if (ret == 0) {
            throw std::runtime_error("Can't write sample. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    } else {
        bool has_sample = false;
        ret = fwrite(&has_sample, sizeof(has_sample), 1, *(server_->channel()));
        if (ret != 1) {
            throw std::runtime_error("Can't write has_sample. Reason: " + std::string(strerror(errno)));
        }
        nbytes += sizeof(has_sample);
    }

    return nbytes;
}

void TreeRpcMulticast::reply(tree_uuid_t uuid, std::shared_ptr<const AddressInfo> addr_info, Method method, const Transaction & transaction,
                             bool has, bool had_something, std::shared_ptr<const ISerializable> value, std::shared_ptr<const statistic::Sample> sample, std::shared_ptr<const Id> id) const {
    std::lock_guard<std::recursive_mutex> lk_channel((server_->channel())->mutex());
    process_reply(method, transaction, uuid, 0, has, had_something, NULL, value.get(), sample.get(), id.get());

    auto addr = std::dynamic_pointer_cast<const AddressInfoMulticast>(addr_info);
    if (addr == NULL) {
        BOOST_LOG_TRIVIAL(trace) << "Replying " << method << " to transaction " << transaction << " to null";
    } else {
        BOOST_LOG_TRIVIAL(trace) << "Replying " << method << " to transaction " << transaction << " to " << inet_ntoa(addr->addr_.sin_addr) << ':' << addr->addr_.sin_port;
    }
    bool force_min_size = false;
    if (method == TreeRpc::kAck || method == TreeRpc::kNack || method == TreeRpc::kAnswerAvailable
            || method == TreeRpc::kAnswerAlloc || method == TreeRpc::kAnswerAllocCheater) {
        force_min_size = true;
    }
    if (method == TreeRpc::kAnswerInsert || method == TreeRpc::kAnswerRemove || method == TreeRpc::kAnswerSearch) {
        if (has == false) {
            force_min_size = true;
        }
    }
    (server_->channel())->sync(addr_info, force_min_size);
}

void TreeRpcMulticast::reply(tree_uuid_t uuid, std::shared_ptr<const AddressInfo> addr_info, Method method, const Transaction & transaction,
                             const Natural & key, std::shared_ptr<const ISerializable> value, std::shared_ptr<const statistic::Sample> sample) {
    std::lock_guard<std::recursive_mutex> lk_channel((server_->channel())->mutex());
    process_reply(method, transaction, uuid, 0, false, false, &key, value.get(), sample.get());

    auto addr = std::dynamic_pointer_cast<const AddressInfoMulticast>(addr_info);
    if (addr == NULL) {
        BOOST_LOG_TRIVIAL(trace) << "Replying " << method << " to transaction " << transaction << " to null";
    } else {
        BOOST_LOG_TRIVIAL(trace) << "Replying " << method << " to transaction " << transaction << " to " << inet_ntoa(addr->addr_.sin_addr) << ':' << addr->addr_.sin_port;
    }
    bool force_min_size = false;
    if (method == TreeRpc::kAck || method == TreeRpc::kNack || method == TreeRpc::kAnswerAvailable
            || method == TreeRpc::kAnswerAlloc || method == TreeRpc::kAnswerAllocCheater) {
        force_min_size = true;
    }
    if (method == TreeRpc::kAnswerSuccessor || method == TreeRpc::kAnswerPredecessor || method == TreeRpc::kAnswerMin || method == TreeRpc::kAnswerMax) {
        if (key.is_valid() == false) {
            force_min_size = true;
        }
    }
    (server_->channel())->sync(addr_info, force_min_size);
}

void TreeRpcMulticast::replyu(tree_uuid_t uuid, std::shared_ptr<const AddressInfo> addr_info, Method method, const Transaction & transaction,
                              bitscnt_t universe, std::shared_ptr<const statistic::Sample> sample) {
    std::lock_guard<std::recursive_mutex> lk_channel((server_->channel())->mutex());
    process_reply(method, transaction, uuid, universe, false, NULL, NULL, sample.get());

    auto addr = std::dynamic_pointer_cast<const AddressInfoMulticast>(addr_info);
    if (addr == NULL) {
        BOOST_LOG_TRIVIAL(trace) << "Replying " << method << " to transaction " << transaction << " to null";
    } else {
        BOOST_LOG_TRIVIAL(trace) << "Replying " << method << " to transaction " << transaction << " to " << inet_ntoa(addr->addr_.sin_addr) << ':' << addr->addr_.sin_port;
    }

    bool force_min_size = false;
    if (method == TreeRpc::kAck || method == TreeRpc::kNack || method == TreeRpc::kAnswerAvailable
            || method == TreeRpc::kAnswerAlloc || method == TreeRpc::kAnswerAllocCheater) {
        force_min_size = true;
    }
    (server_->channel())->sync(addr_info, force_min_size);
}

void TreeRpcMulticast::create_node(bitscnt_t universe, std::shared_ptr<const Id> id) {
    std::vector<ServerDispatchArgs> available_servers;
    volatile bool node_created = false;
    std::condition_variable cond;
    std::mutex mtwait;
    auto sample = statistic::get_thread_sample();

    id_ = id;
    universe_ = universe;

    {
        std::shared_ptr<ObserverHandle<TreeRpc::Method>> observer_avail;
        std::shared_ptr<ObserverHandle<TreeRpc::Method>> observer_allocated;

        observer_avail = server_->add(kAnswerAvailable, [this,&mtwait,&cond,&available_servers](const TreeRpc::Method & method, const ServerDispatchArgs & args) -> bool {
            std::unique_lock<std::mutex> lkwait(mtwait);
            bool handled = false;
            if (method == TreeRpc::kControlForceStop) {
                cond.notify_all();
                return false;
            }
            try {
                if (transaction_ == args.transaction) {
                    handled = true;
                    if (available_servers.size() < Properties::get_instance()->enough_servers_available()) {
                        available_servers.push_back(std::move(args));
                        if (available_servers.size() >= Properties::get_instance()->enough_servers_available()) {
                            cond.notify_all();
                        }
                    }
                    // TODO: put on thread pool ?
                    // this->reply(args.uuid, args.addr, TreeRpc::kAck2, args.transaction, true, nullptr, nullptr, nullptr);
                }
            } catch(const std::runtime_error & e) {
                std::cerr << e.what() << std::endl;
            } catch(...) {
                BOOST_LOG_TRIVIAL(error) << "Got a unknown exception at " << basename(__FILE__) << ":" << __LINE__;
            }
            return handled;
        });

        // Shouldn't happen but lets be defensive
        observer_allocated = server_->add(kAnswerAlloc, [this,&mtwait,&cond,&node_created,&sample](const TreeRpc::Method &method, const ServerDispatchArgs & args) -> bool {
            std::unique_lock<std::mutex> lkwait(mtwait);
            bool handled = false;
            if (method == TreeRpc::kControlForceStop) {
                cond.notify_all();
                return false;
            }
            try {
                if (transaction_ == args.transaction) {
                    if (sample != NULL && args.sample != NULL) {
                        sample->update(*args.sample);
                    }
                    handled = true;
                    node_created = true;
                    cond.notify_all();

                    // TODO: put on thread pool ?
                    BOOST_LOG_TRIVIAL(trace) << "Sending method " << TreeRpc::kAckF << " with transaction " << args.transaction;
                    this->reply(args.uuid, args.addr, TreeRpc::kAckF, args.transaction, true, false, nullptr, nullptr, nullptr);
                }
            } catch(const std::runtime_error & e) {
                std::cerr << e.what() << std::endl;
            } catch(...) {
                BOOST_LOG_TRIVIAL(error) << "Got a unknown exception at " << basename(__FILE__) << ":" << __LINE__;
            }
            return handled;
        });


        {

            std::unique_lock<std::mutex> lkwait(mtwait, std::defer_lock);
            std::unique_lock<std::recursive_mutex> lk_channel((server_->channel())->mutex(), std::defer_lock);

            lkwait.lock();

            uint32_t timeout = 0;

            for(auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                if(true == node_created) {
                    BOOST_LOG_TRIVIAL(trace) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " node created.";
                    return;
                }

                if (available_servers.size() > 0) {
                    break;
                }

                lkwait.unlock();
                lk_channel.lock();
                process(kAvailable, retries == 0);
                BOOST_LOG_TRIVIAL(trace) << "Sending method kAvailable with transaction " << transaction_ << " retries " << retries;
                (server_->channel())->sync();
                lk_channel.unlock();
                lkwait.lock();

                if (sample != NULL) {
                    sample->rmsg_all++;
                }

                //if (available_servers.size() < Properties::get_instance()->enough_servers_available()) {
                if (available_servers.size() == 0) {
                    if (retries > 0) {
                        if (sample != NULL) {
                            sample->rmsg_timeout++;
                        }
                    }
                    BOOST_LOG_TRIVIAL(trace) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " wating for available server. retries " << retries;

                    timeout = FlowControl::timeout();
                    auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                    FlowControl::timeout_feedback(waitres == std::cv_status::timeout);
                    if (server_->exiting()) {
                        throw std::runtime_error("Server is exiting");
                    }
                }

            }

            lkwait.unlock();
            observer_avail.reset();
            observer_allocated.reset();
            lkwait.lock();

            if(true == node_created) {
                BOOST_LOG_TRIVIAL(trace) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " node created.";
                return;
            }

            if (available_servers.size() == 0) {
                BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " got any server response after last timeout of " << timeout;
                throw  exception_peer_not_exist("There is no servers available.");
            } else {
                BOOST_LOG_TRIVIAL(trace) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " got " << available_servers.size() << " available server.";
            }


            while (true) {
                ServerDispatchArgs server;

                if(true == node_created) {
                    BOOST_LOG_TRIVIAL(trace) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " node created.";
                    break;
                }

                if (available_servers.size() == 0) {
                    BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " all server responders falied to really allloc.";
                    throw exception_peer_timedout("None of available servers succeded allocating.");
                }

                // choose one server from the list

                auto server_index = static_cast<uint32_t>(rand()) % available_servers.size();
                std::swap(server, available_servers[server_index]);
                std::dynamic_pointer_cast<AddressInfoMulticast>(server.addr)->addr_.sin_port = htons(Properties::get_instance()->port());
                available_servers.erase(available_servers.begin()+server_index);

                try {
                    auto ret = run_method(TreeRpc::kAlloc, TreeRpc::kAnswerAlloc, nullptr, nullptr, nullptr, server.addr);
                    node_created = true;
                } catch (const exception_peer_not_exist & e) {
                    auto addr = std::dynamic_pointer_cast<AddressInfoMulticast>(server.addr);
                    if (addr != NULL) {
                        BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " . kAlloc failed for " << inet_ntoa(addr->addr_.sin_addr) << ":" << addr->addr_.sin_port << " - " << e.what();
                    } else {
                        BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " . kAlloc failed" << " - " << e.what();
                    }
                } catch (const exception_peer_timedout & e) {
                    auto addr = std::dynamic_pointer_cast<AddressInfoMulticast>(server.addr);
                    if (addr != NULL) {
                        BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " . kAlloc failed for " << inet_ntoa(addr->addr_.sin_addr) << ":" << addr->addr_.sin_port << " - " << e.what();
                    } else {
                        BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " . kAlloc failed" << " - " << e.what();
                    }
                } catch (const std::runtime_error & e) {
                    auto addr = std::dynamic_pointer_cast<AddressInfoMulticast>(server.addr);
                    if (addr != NULL) {
                        BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " . kAlloc failed for " << inet_ntoa(addr->addr_.sin_addr) << ":" << addr->addr_.sin_port << " - " << e.what();
                    } else {
                        BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " . kAlloc failed" << " - " << e.what();
                    }
                } catch (...) {
                    auto addr = std::dynamic_pointer_cast<AddressInfoMulticast>(server.addr);
                    if (addr != NULL) {
                        BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " . kAlloc failed for " << inet_ntoa(addr->addr_.sin_addr) << ":" << addr->addr_.sin_port;
                    } else {
                        BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " . kAlloc failed";
                    }
                }
            }
        }
    }
}

std::tuple<bool, Natural, std::shared_ptr<ISerializable>, bitscnt_t, bool>
TreeRpcMulticast::run_method(TreeRpc::Method method, TreeRpc::Method answer, const Natural * key, const ISerializable * value, const Id * expand_id, std::shared_ptr<const AddressInfo> unicast_addr) const {
    volatile bool ack_received = false;
    volatile bool got_exception = false;
    volatile bool got_non_exist = false;
    volatile bool got_answer = false;
    std::tuple<bool, Natural, std::shared_ptr<ISerializable>, bitscnt_t, bool> ret_value;
    std::condition_variable cond;
    std::mutex mtwait;
    auto sample = statistic::get_thread_sample();

    std::string addr_dest_str;
    auto addr = std::dynamic_pointer_cast<const AddressInfoMulticast>(unicast_addr);
    if (addr == NULL) {
        addr_dest_str = Properties::get_instance()->multicast_group() + ':' + std::to_string(Properties::get_instance()->port());
    } else {
        addr_dest_str = std::string(inet_ntoa(addr->addr_.sin_addr)) + ":" + std::to_string(addr->addr_.sin_port);
    }


    {
        std::shared_ptr<ObserverHandle<TreeRpc::Method>> observer;
        std::shared_ptr<ObserverHandle<TreeRpc::Method>> observer_ack;
        std::shared_ptr<ObserverHandle<TreeRpc::Method>> observer_nack;

        observer = server_->add(answer, [this, &mtwait,&cond,&ret_value,&got_answer,&sample](const TreeRpc::Method &method, const ServerDispatchArgs & args) -> bool {
            std::unique_lock<std::mutex> lkwait(mtwait);
            bool handled = false;
            if (method == TreeRpc::kControlForceStop) {
                cond.notify_all();
                return false;
            }
            try {
                if (transaction_ == args.transaction) {
                    handled = true;
                    ret_value = std::make_tuple(args.has, std::move(args.key), nullptr, args.universe, args.had_something);
                    if (sample != NULL && args.sample != NULL) {
                        sample->update(*args.sample);
                    }
                    got_answer = true;
                    cond.notify_all();

                    // TODO: put on thread pool ?
                    BOOST_LOG_TRIVIAL(trace) << "Sending method " << TreeRpc::kAckF << " with transaction " << args.transaction;
                    this->reply(args.uuid, args.addr, TreeRpc::kAckF, args.transaction, true, false, nullptr, nullptr, nullptr);
                }
            } catch(const std::runtime_error & e) {
                std::cerr << e.what() << std::endl;
            } catch(...) {
            }
            return handled;
        });

        observer_ack = server_->add(kAck, [this,&mtwait,&cond,&ack_received](const TreeRpc::Method &method, const ServerDispatchArgs & args) -> bool {
            std::unique_lock<std::mutex> lkwait(mtwait);
            bool handled = false;
            if (method == TreeRpc::kControlForceStop) {
                cond.notify_all();
                return false;
            }
            try {
                if (transaction_ == args.transaction) {
                    if (false == ack_received) {
                        cond.notify_all();
                        BOOST_LOG_TRIVIAL(trace) << "Got a Ack for "  << args.transaction;
                    }
                    handled = true;
                    ack_received = true;

                    // TODO: put on thread pool ?
                    //BOOST_LOG_TRIVIAL(trace) << "Sending method " << TreeRpc::kAck2 << " with transaction " << args.transaction;
                    //this->reply(args.uuid, args.addr, TreeRpc::kAck2, args.transaction, true, nullptr, nullptr, nullptr);
                }
            } catch(const std::runtime_error & e) {
                std::cerr << e.what() << std::endl;
            } catch(...) {
            }
            return handled;
        });


        observer_nack = server_->add(kNack, [this,&mtwait,&cond,&got_exception,&got_non_exist](const TreeRpc::Method &method, const ServerDispatchArgs & args) -> bool {
            std::unique_lock<std::mutex> lkwait(mtwait);
            bool handled = false;
            if (method == TreeRpc::kControlForceStop) {
                cond.notify_all();
                return false;
            }
            try {
                if (transaction_ == args.transaction) {
                    handled = true;
                    got_exception = args.has;
                    got_non_exist = !got_exception;
                    cond.notify_all();
                    if (got_exception) {
                        BOOST_LOG_TRIVIAL(warning) << "Got a Nack exception for " << args.transaction;
                    } else {
                        BOOST_LOG_TRIVIAL(trace) << "Got a Nack not exist for "  << args.transaction;
                    }

                    // TODO: put on thread pool ?
                    //BOOST_LOG_TRIVIAL(trace) << "Sending method " << TreeRpc::kAck2 << " with transaction " << args.transaction;
                    //this->reply(args.uuid, args.addr, TreeRpc::kAck2, args.transaction, true, nullptr, nullptr, nullptr);
                }
            } catch(const std::runtime_error & e) {
                std::cerr << e.what() << std::endl;
            } catch(...) {
            }
            return handled;
        });

        {
            std::unique_lock<std::mutex> lkwait(mtwait, std::defer_lock);
            std::unique_lock<std::recursive_mutex> lk_channel((server_->channel())->mutex(), std::defer_lock);

            lkwait.lock();

            uint32_t timeout = 0;

            for(auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                lkwait.unlock();
                lk_channel.lock();
                process(method, retries == 0, key, value, expand_id);
                BOOST_LOG_TRIVIAL(trace) << "Sending method " << method << " with transaction " << transaction_ << " retry " << retries;
                (server_->channel())->sync(unicast_addr);
                lk_channel.unlock();
                lkwait.lock();

                if (sample != NULL) {
                    sample->rmsg_all++;
                }

                if (got_exception) {
                    BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " peer Nack exception";
                    throw std::runtime_error("Peer got an exception");
                }
                if (got_non_exist) {
                    BOOST_LOG_TRIVIAL(trace) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " peer Nack doesnt exists";
                    throw  exception_peer_not_exist();
                }

                if (ack_received == false) {
                    if (retries > 0) {
                        if (sample != NULL) {
                            sample->rmsg_timeout++;
                        }
                    }
                    BOOST_LOG_TRIVIAL(trace) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " wating for ack";

                    timeout = FlowControl::timeout();
                    auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                    FlowControl::timeout_feedback(waitres == std::cv_status::timeout);
                    if (server_->exiting()) {
                        throw std::runtime_error("Server is exiting");
                    }
                } else {
                    break;
                }

                if (got_exception) {
                    BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " peer Nack exception";
                    throw std::runtime_error("Peer got an exception");
                }
                if (got_non_exist) {
                    BOOST_LOG_TRIVIAL(trace) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " peer Nack doesnt exists";
                    throw  exception_peer_not_exist();
                }
                if (ack_received) {
                    break;
                }

            }

            if (ack_received == false) {
                if (Properties::get_instance()->self_cheating() || Properties::get_instance()->has_cheater()) {
                    BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " got no ack after last timeout of " << timeout << " ms";
                }
                throw exception_peer_timedout("We got no Ack");
            }

            lkwait.unlock();
            observer_ack.reset();
            lkwait.lock();


            if (got_answer == false) {
                std::cv_status waitres;

                if (got_exception) {
                    BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " peer Nack exception after the ack";
                    throw std::runtime_error("Peer got an exception");
                }
                if (got_non_exist) {
                    BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " peer Nack doesnt exists after the ack";
                    throw  std::runtime_error("Weird. Peer sent ack but answer. Actually we got 'got_non_exist' after an Ack");
                }

                timeout = FlowControl::long_timeout(universe_);

                BOOST_LOG_TRIVIAL(trace) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " got ack but result. Lets wait for " << timeout << " ms";
                waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                if (server_->exiting()) {
                    throw std::runtime_error("Server is exiting");
                }

                if (got_answer == false) {
                    BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " long wait of " << timeout << " ms finished without answer and with " << (std::cv_status::timeout == waitres ? "timeout" : "result");
                    if (got_exception) {
                        BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " peer Nack exception after long wait of " << timeout << " ms";
                        throw std::runtime_error("Peer got an exception");
                    }
                    if (got_non_exist) {
                        BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " peer Nack doesnt exists after long wait of " << timeout << " ms";
                        throw  std::runtime_error("Weird. Peer sent ack but answer. Actually we got 'got_non_exist' after an Ack and long wait");
                    }

                    BOOST_LOG_TRIVIAL(error) << "Universe " << universe_ << " Id " << *id_ << " Transaction " << transaction_ << " Method " << method << " to " << addr_dest_str << " after long wait of " << timeout << " ms, got nothing";
                    throw std::runtime_error("Peer ack and then not responded");
                }

            }
        }
    }

    return ret_value;
}

std::tuple<bool, std::shared_ptr<ISerializable>,bool> TreeRpcMulticast::insert(const Natural & key, std::shared_ptr<ISerializable> value) {
    auto ret = run_method(TreeRpc::kInsert, TreeRpc::kAnswerInsert, &key, value.get());
    return std::make_tuple(std::get<0>(ret), std::get<2>(ret), std::get<4>(ret));
}

std::tuple<bool, std::shared_ptr<ISerializable>,bool> TreeRpcMulticast::remove(const Natural & key) {
    auto ret = run_method(TreeRpc::kRemove, TreeRpc::kAnswerRemove, &key);
    return std::make_tuple(std::get<0>(ret), std::get<2>(ret), std::get<4>(ret));
}

std::pair<bool, std::shared_ptr<ISerializable>> TreeRpcMulticast::search(const Natural & key) {
    auto ret = run_method(TreeRpc::kSearch, TreeRpc::kAnswerSearch, &key);
    return std::make_pair(std::get<0>(ret), std::get<2>(ret));
}

std::pair<Natural, std::shared_ptr<ISerializable>> TreeRpcMulticast::successor(const Natural & key) {
    auto ret = run_method(TreeRpc::kSuccessor, TreeRpc::kAnswerSuccessor, &key);
    return std::make_pair(std::get<1>(ret), std::get<2>(ret));
}

std::pair<Natural, std::shared_ptr<ISerializable>> TreeRpcMulticast::predecessor(const Natural & key) {
    //std::cout << "sending predecessor of " << std::hex<< static_cast<uint32_t>(key) << std::endl;
    auto ret = run_method(TreeRpc::kPredecessor, TreeRpc::kAnswerPredecessor, &key);
    //std::cout << "ret sending predecessor of " << std::hex << static_cast<uint32_t>(std::get<1>(ret)) << std::endl;
    return std::make_pair(std::get<1>(ret), std::get<2>(ret));
}

std::pair<Natural, std::shared_ptr<ISerializable>> TreeRpcMulticast::min() const {
    auto ret = run_method(TreeRpc::kMin, TreeRpc::kAnswerMin);
    return std::make_pair(std::get<1>(ret), std::get<2>(ret));
}

void TreeRpcMulticast::touch() {
    throw std::runtime_error("Implement it yourself.");
}

std::pair<Natural, std::shared_ptr<ISerializable>> TreeRpcMulticast::max() const {
    auto ret = run_method(TreeRpc::kMax, TreeRpc::kAnswerMax);
    return std::make_pair(std::get<1>(ret), std::get<2>(ret));
}

bitscnt_t TreeRpcMulticast::universe() {
    auto ret = run_method(TreeRpc::kUniverse, TreeRpc::kAnswerUniverse);
    return std::get<3>(ret);
}

void TreeRpcMulticast::veb_expanded(std::shared_ptr<ITree> tree) {
    run_method(TreeRpc::kVeb_expanded, TreeRpc::kAnswerVeb_expanded, NULL, NULL, tree->id().get());
}

size_t TreeRpcMulticast::parse(TreeRpc::Method & method, ServerDispatchArgs & sda, const std::vector<uint8_t> & buffer, bool header_only) {
    size_t nbytes = 0;
    size_t ret = 0;
    FILE * aux = fmemopen(const_cast<uint8_t *>(buffer.data()), buffer.size(), "rb");
    if (aux == NULL) {
        throw std::runtime_error("Can't map buffer into FILE. Reason: " + std::string(strerror(errno)));
    }
    std::shared_ptr<FILE> file(aux, ::fclose);
    aux = NULL;

    ret = fread(&method, sizeof(method), 1, file.get());
    if (ret != 1) {
        throw std::runtime_error("Can't read method name. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(method);

    ret = fread(&sda.port, sizeof(sda.port), 1, file.get());
    if (ret != 1) {
        throw std::runtime_error("Can't read port. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(sda.port);

    ret = fread(&sda.uuid, sizeof(sda.uuid), 1, file.get());
    if (ret != 1) {
        throw std::runtime_error("Can't read uuid. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(sda.uuid);

    ret = sda.transaction.read(file.get());
    if (ret <= 0) {
        throw std::runtime_error("Can't read transaction.");
    }
    nbytes += ret;

    ret = fread(&sda.universe, sizeof(sda.universe), 1, file.get());
    if (ret != 1) {
        throw std::runtime_error("Can't read universe. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(sda.universe);

    if (header_only) {
        return nbytes;
    }

    if (method >= TreeRpc::kMethodBegin && method < TreeRpc::kMethodEnd) {
        nbytes += parse_method(sda, file.get());
    } else if (method >= TreeRpc::kAnswerBegin && method < TreeRpc::kAnswerEnd) {
        nbytes += parse_reply(sda, file.get());
    } else if (method == TreeRpc::kAck || /* method == TreeRpc::kAck2 || */ method == TreeRpc::kAckF || method == TreeRpc::kNack) {
        nbytes += parse_reply(sda, file.get());
    } else {
        if (method == TreeRpc::kAvailable || method == TreeRpc::kAlloc) {
            nbytes += parse_method(sda, file.get());
        } else if (method == TreeRpc::kAnswerAllocCheater) {
            nbytes += parse_reply(sda, file.get());
        } else {
            BOOST_LOG_TRIVIAL(error) << "Parse not implemented for " << static_cast<int32_t>(method);
            throw std::runtime_error("Parse not implemented for " + std::to_string(static_cast<int32_t>(method)));
        }
    }

    return nbytes;
}

size_t TreeRpcMulticast::parse_reply(ServerDispatchArgs & sda, FILE *stream) {
    size_t nbytes = 0;
    size_t ret = 0;

    ret = fread(&sda.has, sizeof(sda.has), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read has. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(sda.has);

    ret = fread(&sda.had_something, sizeof(sda.had_something), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read had_something. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(sda.had_something);

    bool has_key = false;
    ret = fread(&has_key, sizeof(has_key), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read has_key. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(has_key);

    if (has_key) {
        ret = sda.key.read(stream);
        if (ret <= 0) {
            throw std::runtime_error("Can't read Key. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    }

    bool has_value = false;
    ret = fread(&has_value, sizeof(has_value), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read has_value. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(has_value);

    if (has_value) {
        throw std::runtime_error("Not implemented");
    }

    bool has_sample = false;
    ret = fread(&has_sample, sizeof(has_sample), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read has_sample. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(has_sample);

    if (has_sample) {
        sda.sample = std::make_shared<statistic::Sample>();
        ret = sda.sample->read(stream);
        if (ret <= 0) {
            throw std::runtime_error("Can't read sample. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    }

    bool has_id = false;
    ret = fread(&has_id, sizeof(has_id), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read has_id. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(has_id);

    if (has_id) {
        sda.id = std::make_shared<Id>(stream, &ret);
        if (ret <= 0) {
            throw std::runtime_error("Can't read Id. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    }

    return nbytes;
}

size_t TreeRpcMulticast::parse_method(ServerDispatchArgs & sda, FILE *stream) {
    size_t nbytes = 0;
    size_t ret = 0;

    ret = fread(&sda.to_root, sizeof(sda.to_root), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read to_root. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(sda.to_root);

    if (!sda.to_root) {
        sda.id = std::make_shared<Id>(stream, &ret);
        if (ret <= 0) {
            throw std::runtime_error("Can't read Id. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    }

    bool has_key = false;
    ret = fread(&has_key, sizeof(has_key), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read has_key. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(has_key);

    if (has_key) {
        ret = sda.key.read(stream);
        if (ret <= 0) {
            throw std::runtime_error("Can't read Key. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    }

    bool has_value = false;
    ret = fread(&has_value, sizeof(has_value), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read has_value. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(has_value);


    if (has_value) {
        throw std::runtime_error("Not implemented");
    }

    bool has_expand_tree_id = false;
    ret = fread(&has_expand_tree_id, sizeof(has_expand_tree_id), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read has_expand_tree_id. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(has_expand_tree_id);

    if (has_expand_tree_id) {
        ret = sda.expand_tree_id.read(stream);
        if (ret <= 0) {
            throw std::runtime_error("Can't read expand_tree_id. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    }

    bool has_sample = false;
    ret = fread(&has_sample, sizeof(has_sample), 1, stream);
    if (ret != 1) {
        throw std::runtime_error("Can't read has_sample. Reason: " + std::string(strerror(errno)));
    }
    nbytes += sizeof(has_sample);

    if (has_sample) {
        sda.sample = std::make_shared<statistic::Sample>();
        ret = sda.sample->read(stream);
        if (ret <= 0) {
            throw std::runtime_error("Can't read sample. Reason: " + std::string(strerror(errno)));
        }
        nbytes += ret;
    }

    return nbytes;
}

}
