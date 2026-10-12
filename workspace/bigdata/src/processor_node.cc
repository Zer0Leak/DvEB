
#include <boost/log/trivial.hpp>

#include "bigdata/processor_node.hh"
#include "bigdata/ichannel.hh"
#include "bigdata/server.hh"
#include "bigdata/registry.hh"
#include "bigdata/factory.hh"
#include "bigdata/tree_rpc_multicast.hh"
#include "bigdata/veb/iveb_global.hh"
#include "bigdata/util.hh"
#include "bigdata/properties.hh"
#include "bigdata/statistc.hh"
#include "bigdata/flow_control.hh"

namespace bigdata {

ProcessorNode::ProcessorNode(std::shared_ptr<Registry> registry, std::shared_ptr<Factory> factory)
    : registry_(registry), factory_(factory)
{
}

#include <iostream>

void ProcessorNode::reply_ack(TreeRpcMulticast & trpc, const ServerDispatchArgs & sda, std::shared_ptr<Server> server, TreeRpc::Method method, bool has) {

    trpc.reply(sda.uuid, sda.addr, method, sda.transaction, has, false, nullptr, nullptr, nullptr);

#if 0

    volatile bool ack2_received = false;
    std::condition_variable cond;
    std::mutex mtwait;

    {
        std::shared_ptr<ObserverHandle<TreeRpc::Method>> observer_ack2;

        observer_ack2 = server->add(TreeRpc::kAck2, [this,&mtwait,&cond,&ack2_received,&sda](const TreeRpc::Method &method, const ServerDispatchArgs & args) -> bool {
            std::unique_lock<std::mutex> lkwait(mtwait);

            bool handled = false;
            if (method == TreeRpc::kControlForceStop) {
                cond.notify_all();
                return false;
            }
            try {
                if (sda.transaction == args.transaction) {
                    handled = true;
                    ack2_received = true;
                    cond.notify_all();
                }
            } catch(const std::runtime_error & e) {
                std::cerr << e.what() << std::endl;
            } catch(...) {
                BOOST_LOG_TRIVIAL(error) << "Got a unknown exception at " << basename(__FILE__) << ":" << __LINE__;
            }
            return handled;
        });

        {

            std::unique_lock<std::mutex> lkwait(mtwait);

            for(auto retries=0; ack2_received == false; retries++) {
                if (retries <= Properties::get_instance()->retries()) {
                    if (retries == 0) {
                        BOOST_LOG_TRIVIAL(trace) << "Sending " << method << " to " << sda.transaction << " retries " << retries;
                    } else {
                        BOOST_LOG_TRIVIAL(warning) << "Sending " << method << " to " << sda.transaction << " retries " << retries;
                    }

                    if (retries > 0) {
                        auto sample = statistic::get_thread_sample();
                        if (sample != NULL) {
                            sample->rmsg_timeout++;
                        }
                    }

                    trpc.reply(sda.uuid, sda.addr, method, sda.transaction, has, nullptr, nullptr, nullptr);
                    if (ack2_received == false) {
                        auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(FlowControl::timeout()));
                        FlowControl::timeout_feedback(waitres == std::cv_status::timeout);
                    } else {
                        break;
                    }
                    if (server->exiting()) {
                        throw std::runtime_error("Server is exiting");
                    }
                } else {
                    auto addr = std::dynamic_pointer_cast<AddressInfoMulticast>(sda.addr);
                    if (addr == NULL) {
                        BOOST_LOG_TRIVIAL(error) << "Got no " << TreeRpc::kAck2 << " for " << sda.transaction << " null addr";
                    } else {
                        BOOST_LOG_TRIVIAL(error) << "Got no " << TreeRpc::kAck2 << " for " << sda.transaction << " from " << inet_ntoa(addr->addr_.sin_addr) << ":" << addr->addr_.sin_port;
                    }
                    throw std::runtime_error("Got no Ack2");
                }
            }

        }

    }

#endif
}

bool ProcessorNode::process_send_cheater(std::shared_ptr<Server> server, const ServerDispatchArgs & sda, std::mutex & mutexf, std::condition_variable & cond) {
    Transaction trans(server->channel()->mac_address());

    volatile bool ackfcheater_received = false;
    volatile bool force_received = false;
    auto id = std::make_shared<bigdata::Id>();
    bigdata::TreeRpcMulticast trpc(sda.universe > 1 ? sda.universe : 1, server, sda.uuid, id, false);

    {
        auto observer_ackfcheater = server->add(TreeRpc::kAckF, [this,&mutexf,&cond,&ackfcheater_received,&force_received,&trans](const TreeRpc::Method &method, const ServerDispatchArgs & args) -> bool {
            std::unique_lock<std::mutex> lkwait(mutexf);
            bool handled = false;
            if (method == TreeRpc::kControlForceStop) {
                force_received = true;
                cond.notify_all();
                return false;
            }
            try {
                if (trans == args.transaction) {
                    handled = true;
                    ackfcheater_received = true;
                    cond.notify_all();
                }
            } catch(const std::runtime_error & e) {
                std::cerr << e.what() << std::endl;
            } catch(...) {
                BOOST_LOG_TRIVIAL(error) << "Got a unknown exception at " << basename(__FILE__) << ":" << __LINE__;
            }
            return handled;
        });

        {
            std::unique_lock<std::mutex> lkwait(mutexf);

            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {

                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }

                trpc.reply(sda.uuid, nullptr, TreeRpc::kAnswerAllocCheater, trans, true, false, nullptr, statistic::get_thread_sample(), sda.id);

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout), [&ackfcheater_received, &force_received]() -> bool {
                    return ackfcheater_received || force_received;
                });
                FlowControl::timeout_feedback(!ackfcheater_received);
                if (ackfcheater_received) {
                    break;
                } else {
                    if (waitres == true && ackfcheater_received == false && force_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << TreeRpc::kAnswerAllocCheater << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << TreeRpc::kAnswerAllocCheater << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == false ? " timed out " : " awaken");
                }
            }
        }
    }

    return ackfcheater_received;
}

void ProcessorNode::process_avail(bool active_transaction, std::shared_ptr<Server> server, const ServerDispatchArgs & sda, std::mutex & mutexf, std::condition_variable & cond, volatile bool & ackf_received) {
    auto id = std::make_shared<bigdata::Id>();
    bigdata::TreeRpcMulticast trpc(sda.universe > 1 ? sda.universe : 1, server, sda.uuid, id, false);

    std::shared_ptr<ITree> tree;

    if (Properties::get_instance()->no_hosting()) {
        return;
    }

    tree = registry_->get(sda.uuid, sda.id);

    if (tree != NULL) {
        if (active_transaction == false) {
            if (Properties::get_instance()->has_cheater()) {
                if (!process_send_cheater(server, sda, mutexf, cond)) {
                    return;
                }
            }
            std::unique_lock<std::mutex> lkwait(mutexf);

            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }
                trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerAlloc, sda.transaction, true, false, nullptr, statistic::get_thread_sample(), sda.id);

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                FlowControl::timeout_feedback(!ackf_received);
                if (ackf_received) {
                    break;
                } else {
                    if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << TreeRpc::kAnswerAlloc << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << TreeRpc::kAnswerAlloc << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                }
            }
        }
    } else {
        auto avail = get_available_memory() * 100ull / get_total_memory();
        if (avail >= Properties::get_instance()->memory_threshold()) {
            // reply_ack(trpc, sda, server, TreeRpc::kAnswerAvailable, true);
            trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerAvailable, sda.transaction, true, false, nullptr, nullptr, nullptr);
        } else {
            BOOST_LOG_TRIVIAL(warning) << "Available memory is low: " << avail << " for " << TreeRpc::kAnswerAvailable << " " << sda.transaction;
        }
    }
}

void ProcessorNode::process_alloc(bool active_transaction, std::shared_ptr<Server> server, const ServerDispatchArgs & sda, std::mutex & mutexf, std::condition_variable & cond, volatile bool & ackf_received) {
    auto id = std::make_shared<bigdata::Id>();
    bigdata::TreeRpcMulticast trpc(sda.universe > 1 ? sda.universe : 1, server, sda.uuid, id, false);

    try {
        reply_ack(trpc, sda, server, TreeRpc::kAck, true);
    } catch (const std::runtime_error & e) {
        BOOST_LOG_TRIVIAL(error) << "Exception processing a method call " << TreeRpc::kAlloc << " " << sda.transaction << " : " << e.what();
        return;
    } catch (...) {
        BOOST_LOG_TRIVIAL(error) << "Exception processing a method call " << TreeRpc::kAlloc << " " << sda.transaction;
        return;
    }

    if (active_transaction) return;

    try {
        std::shared_ptr<ITree> tree;
        std::unique_lock<std::recursive_mutex> lckg(registry_->mutex());
        tree = registry_->get(sda.uuid, sda.id);

        if (tree != NULL) {
            if (registry_->get_root(sda.uuid) == NULL) {
                registry_->sign(sda.uuid, nullptr);
            }
            lckg.unlock();

            if (Properties::get_instance()->has_cheater()) {
                if (!process_send_cheater(server, sda, mutexf, cond)) {
                    return;
                }
            }

            std::unique_lock<std::mutex> lkwait(mutexf);

            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }
                trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerAlloc, sda.transaction, true, false, nullptr, statistic::get_thread_sample(), sda.id);

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                FlowControl::timeout_feedback(!ackf_received);
                if (ackf_received) {
                    break;
                } else {
                    if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << TreeRpc::kAnswerAlloc << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << TreeRpc::kAnswerAlloc << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                }
            }

        } else {
            if (registry_->get_root(sda.uuid) == NULL) {
                registry_->sign(sda.uuid, nullptr);
            }

            BOOST_LOG_TRIVIAL(trace) << "Creating remote tree. universe " << sda.universe;
            tree = std::get<2>(factory_->create(sda.uuid, sda.universe, sda.id, server, Factory::kTreeAuto, Factory::kInstanceRemote));
            BOOST_LOG_TRIVIAL(trace) << "Let's check it it has been created. universe " << sda.universe;

            statistic::thread_sample_set_level(sda.universe);

            if (tree != NULL) {
                registry_->sign(sda.uuid, sda.id, tree);
                lckg.unlock();

                tree->touch();


                BOOST_LOG_TRIVIAL(trace) << "kAnswerAlloc replying.";

                if (Properties::get_instance()->has_cheater()) {
                    if (!process_send_cheater(server, sda, mutexf, cond)) {
                        return;
                    }
                }

                std::unique_lock<std::mutex> lkwait(mutexf);

                for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                    if (retries > 0) {
                        auto sample = statistic::get_thread_sample();
                        if (sample != NULL) {
                            sample->rmsg_timeout++;
                        }
                    }
                    trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerAlloc, sda.transaction, true, false, nullptr, statistic::get_thread_sample(), sda.id);

                    auto timeout = FlowControl::timeout();
                    auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                    FlowControl::timeout_feedback(!ackf_received);
                    if (ackf_received) {
                        break;
                    } else {
                        if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                            BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << TreeRpc::kAnswerAlloc << " in " << timeout << " ms " << " for " << sda.transaction;
                        }
                        BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << TreeRpc::kAnswerAlloc << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                    }
                }
            } else {
                lckg.unlock();
                BOOST_LOG_TRIVIAL(error) << "kAlloc failed." << " for " << sda.transaction;
                reply_ack(trpc, sda, server, TreeRpc::kNack, true);
            }
        }
    } catch (const std::runtime_error & e) {
        BOOST_LOG_TRIVIAL(error) << "kAlloc failed for " << sda.transaction <<". Let's return a Nack: " << e.what();
        reply_ack(trpc, sda, server, TreeRpc::kNack, true);
    } catch (...) {
        BOOST_LOG_TRIVIAL(error) << "kAlloc failed for " << sda.transaction << ". Let's return a Nack.";
        reply_ack(trpc, sda, server, TreeRpc::kNack, true);
    }
}

void ProcessorNode::process_method(bool active_transaction, std::shared_ptr<Server> server, const TreeRpc::Method method, const ServerDispatchArgs & sda, std::mutex & mutexf, std::condition_variable & cond, volatile bool & ackf_received) {
    auto id = std::make_shared<bigdata::Id>();
    bigdata::TreeRpcMulticast trpc(sda.universe > 1 ? sda.universe : 1, server, sda.uuid, id, false);

    std::shared_ptr<ITree> tree;

    if (sda.to_root) {
        tree = registry_->get_root(sda.uuid);
        if (tree == NULL) {
            BOOST_LOG_TRIVIAL(trace) << "We don't own root " << sda.uuid << " requested in " << sda.transaction << " for " << method;
            return;
        }
    } else {
        tree = registry_->get(sda.uuid, sda.id);
        if (tree == NULL) {
            BOOST_LOG_TRIVIAL(trace) << "We don't own node of " << sda.uuid << " requested in " << sda.transaction << " for " << method << " id " << *sda.id;
            if (Properties::get_instance()->self_cheating()) {
                try {
                    reply_ack(trpc, sda, server, TreeRpc::kNack, false);
                } catch (const std::runtime_error & e) {
                    BOOST_LOG_TRIVIAL(error) << "Exception processing a method call replying nack " << method << " " << sda.transaction << " : " << e.what();
                    return;
                } catch (...) {
                    BOOST_LOG_TRIVIAL(error) << "Exception processing a method call replying nack " << method << " " << sda.transaction;
                    return;
                }
            }
            return;
        }
    }


    try {
        reply_ack(trpc, sda, server, TreeRpc::kAck, true);
    } catch (const std::runtime_error & e) {
        BOOST_LOG_TRIVIAL(error) << "Exception processing a method call replying Ack" << method << " " << sda.transaction << " : " << e.what();
        return;
    } catch (...) {
        BOOST_LOG_TRIVIAL(error) << "Exception processing a method call replying Ack" << method << " " << sda.transaction;
        return;
    }

    if (active_transaction) return;

    try {
        switch (method) {
        case TreeRpc::kInsert: {
            auto r = tree->insert(sda.key, nullptr);

            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                std::unique_lock<std::mutex> lkwait(mutexf);
                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }
                trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerInsert, sda.transaction, std::get<0>(r), std::get<2>(r), std::get<1>(r), statistic::get_thread_sample(), nullptr);

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                FlowControl::timeout_feedback(!ackf_received);
                if (ackf_received) {
                    break;
                } else {
                    if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << method << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << method << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                }
            }
        }
            break;
        case TreeRpc::kRemove: {
            auto r = tree->remove(sda.key);
            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                std::unique_lock<std::mutex> lkwait(mutexf);
                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }
                trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerRemove, sda.transaction, std::get<0>(r), std::get<2>(r), std::get<1>(r), statistic::get_thread_sample(), nullptr);

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                FlowControl::timeout_feedback(!ackf_received);
                if (ackf_received) {
                    break;
                } else {
                    if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << method << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << method << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                }
            }

        }
            break;
        case TreeRpc::kSearch: {
            auto r = tree->search(sda.key);
            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                std::unique_lock<std::mutex> lkwait(mutexf);
                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }
                trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerSearch, sda.transaction, r.first, false, r.second, statistic::get_thread_sample(), nullptr);

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                FlowControl::timeout_feedback(!ackf_received);
                if (ackf_received) {
                    break;
                } else {
                    if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << method << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << method << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                }
            }

        }
            break;
        case TreeRpc::kSuccessor: {
            auto r = tree->successor(sda.key);
            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                std::unique_lock<std::mutex> lkwait(mutexf);
                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }
                trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerSuccessor, sda.transaction, r.first, r.second, statistic::get_thread_sample());

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                FlowControl::timeout_feedback(!ackf_received);
                if (ackf_received) {
                    break;
                } else {
                    if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << method << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << method << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                }
            }

        }
            break;
        case TreeRpc::kPredecessor: {
            auto r = tree->predecessor(sda.key);
            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                std::unique_lock<std::mutex> lkwait(mutexf);
                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }
                trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerPredecessor, sda.transaction, r.first, r.second, statistic::get_thread_sample());

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                FlowControl::timeout_feedback(!ackf_received);
                if (ackf_received) {
                    break;
                } else {
                    if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << method << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << method << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                }
            }

        }
            break;
        case TreeRpc::kMin: {
            auto r = tree->min();
            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                std::unique_lock<std::mutex> lkwait(mutexf);
                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }
                trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerMin, sda.transaction, r.first, r.second, statistic::get_thread_sample());

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                FlowControl::timeout_feedback(!ackf_received);
                if (ackf_received) {
                    break;
                } else {
                    if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << method << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << method << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                }
            }

        }
            break;
        case TreeRpc::kMax: {
            auto r = tree->max();
            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                std::unique_lock<std::mutex> lkwait(mutexf);
                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }
                trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerMax, sda.transaction, r.first, r.second, statistic::get_thread_sample());

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                FlowControl::timeout_feedback(!ackf_received);
                if (ackf_received) {
                    break;
                } else {
                    if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << method << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << method << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                }
            }

        }
            break;
        case TreeRpc::kUniverse: {
            auto r = tree->universe();
            for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                std::unique_lock<std::mutex> lkwait(mutexf);
                if (retries > 0) {
                    auto sample = statistic::get_thread_sample();
                    if (sample != NULL) {
                        sample->rmsg_timeout++;
                    }
                }
                trpc.replyu(sda.uuid, sda.addr, TreeRpc::kAnswerUniverse, sda.transaction, r, statistic::get_thread_sample());

                auto timeout = FlowControl::timeout();
                auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                FlowControl::timeout_feedback(!ackf_received);
                if (ackf_received) {
                    break;
                } else {
                    if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                        BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << method << " in " << timeout << " ms " << " for " << sda.transaction;
                    }
                    BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << method << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                }
            }

        }
            break;
        case TreeRpc::kVeb_expanded: {
            auto iveb = std::dynamic_pointer_cast<veb::IVebGlobal>(tree);
            statistic::thread_sample_set_level(sda.universe);

            if (iveb != NULL) {
                auto expanded_tree = registry_->get(sda.uuid, sda.expand_tree_id);
                if (expanded_tree == NULL) {
                    expanded_tree = std::get<2>(factory_->create(sda.uuid, sda.universe, std::make_shared<Id>(std::move(sda.expand_tree_id)), server, Factory::kTreeAuto, Factory::kInstanceClient));
                    expanded_tree->touch();
                }
                iveb->expanded(expanded_tree);

                for (auto retries=0; retries <= Properties::get_instance()->retries(); retries++) {
                    std::unique_lock<std::mutex> lkwait(mutexf);
                    if (retries > 0) {
                        auto sample = statistic::get_thread_sample();
                        if (sample != NULL) {
                            sample->rmsg_timeout++;
                        }
                    }
                    trpc.reply(sda.uuid, sda.addr, TreeRpc::kAnswerVeb_expanded, sda.transaction, true, false, nullptr, statistic::get_thread_sample(), nullptr);

                    auto timeout = FlowControl::timeout();
                    auto waitres = cond.wait_for(lkwait, std::chrono::milliseconds(timeout));
                    FlowControl::timeout_feedback(!ackf_received);
                    if (ackf_received) {
                        break;
                    } else {
                        if (waitres == std::cv_status::no_timeout && ackf_received == false) {
                            BOOST_LOG_TRIVIAL(error) << "Awaken somewhere else " << method << " in " << timeout << " ms " << " for " << sda.transaction;
                        }
                        BOOST_LOG_TRIVIAL(debug) << "Ackf not received for " << method << " in " << timeout << " ms " << " for " << sda.transaction << " waitres " << (waitres == std::cv_status::timeout ? " timed out " : " awaken");
                    }
                }

            }
        }
            break;
        default:
            break;
        }

    } catch (const std::runtime_error & e) {
        BOOST_LOG_TRIVIAL(error) << "Exception processing a method call " << method << " " << sda.transaction << " : " << e.what();
        reply_ack(trpc, sda, server, TreeRpc::kNack, true);
    } catch (...) {
        BOOST_LOG_TRIVIAL(error) << "Unknown exception processing a method call " << method << " " << sda.transaction;
        reply_ack(trpc, sda, server, TreeRpc::kNack, true);
    }
}

void ProcessorNode::process(std::vector<uint8_t> buffer, std::shared_ptr<AddressInfo> addr, std::shared_ptr<Server> server) {
    std::mutex mutexf;
    std::condition_variable cond;
    volatile bool ackf_received = false;
    std::shared_ptr<void> active_trans;


    TreeRpc::Method method = TreeRpc::kInvalid;
    ServerDispatchArgs sda;
    TreeRpcMulticast::parse(method, sda, buffer);
    swap(sda.addr, addr);

    if (method >= 0 && method < TreeRpc::kEnd) {
        auto addr = std::dynamic_pointer_cast<AddressInfoMulticast>(sda.addr);
        if (addr == NULL) {
            BOOST_LOG_TRIVIAL(trace) << "Received " << method << " for " << sda.transaction << " Key " << sda.key << " universe " << sda.universe << " null addr";
        } else {
            BOOST_LOG_TRIVIAL(trace) << "Received " << method << " for " << sda.transaction << " Key " << sda.key << " universe " << sda.universe << " from " << inet_ntoa(addr->addr_.sin_addr) << ":" << addr->addr_.sin_port;
        }
    }

    {
        std::unique_lock<std::recursive_mutex> active_trans_lg(active_tranasctions_.mutex());
        if (active_tranasctions_.has(sda.transaction)) {
            if (method >= TreeRpc::kMethodBegin && method < TreeRpc::kMethodEnd) {
                active_trans_lg.unlock();
                if (! sda.transaction.am_i_owner(server->channel()->mac_address()) ) {
                    BOOST_LOG_TRIVIAL(debug) << "already active " << method << " for " << sda.transaction;
                }
                process_method(true, server, method, sda, mutexf, cond, ackf_received);
                return;
            } else {
                if (method >= TreeRpc::kAvailable) {
                    active_trans_lg.unlock();
                    if (! sda.transaction.am_i_owner(server->channel()->mac_address()) ) {
                        BOOST_LOG_TRIVIAL(debug) << "already active " << method << " for " << sda.transaction;
                    }
                    process_avail(true, server, sda, mutexf, cond, ackf_received);
                    return;
                } else if (method >= TreeRpc::kAlloc) {
                    active_trans_lg.unlock();
                    if (! sda.transaction.am_i_owner(server->channel()->mac_address()) ) {
                        BOOST_LOG_TRIVIAL(debug) << "already active " << method << " for " << sda.transaction;
                    }
                    process_alloc(true, server, sda, mutexf, cond, ackf_received);
                    return;
                }
            }
        } else {
            active_tranasctions_.insert(sda.transaction);
            Transaction trans(sda.transaction);
            active_trans = std::shared_ptr<void>(NULL, [this, trans](void*){
                this->active_tranasctions_.remove(trans);
            });

        }
        active_trans_lg.unlock();

        std::dynamic_pointer_cast<AddressInfoMulticast>(sda.addr)->addr_.sin_port = htons(sda.port);

        auto notified = server->notify(method, sda);

        if (!notified) {
            auto id = std::make_shared<bigdata::Id>();
            bigdata::TreeRpcMulticast trpc(sda.universe > 1 ? sda.universe : 1, server, sda.uuid, id, false);

            if (method == TreeRpc::kAck) {
                if (sda.transaction.am_i_owner(server->channel()->mac_address()) ) {
                    /*
                auto addr = std::dynamic_pointer_cast<AddressInfoMulticast>(sda.addr);
                if (addr == NULL) {
                    BOOST_LOG_TRIVIAL(debug) << "Replying lost " << TreeRpc::kAck2 << " to " << method << " " << sda.transaction << " null addr";
                } else {
                    BOOST_LOG_TRIVIAL(debug) << "Replying lost " << TreeRpc::kAck2 << " to " << method << " " << sda.transaction << " from " << inet_ntoa(addr->addr_.sin_addr) << ":" << addr->addr_.sin_port;
                }
                trpc.reply(sda.uuid, sda.addr, TreeRpc::kAck2, sda.transaction, false, nullptr, nullptr, nullptr);
                */
                    return;
                }
            } else if (method >= TreeRpc::kAnswerBegin && method < TreeRpc::kAnswerEnd) {
                if (sda.transaction.am_i_owner(server->channel()->mac_address()) ) {
                    if (method == TreeRpc::kAnswerAvailable) {
                        return;
                    }
                    auto addr = std::dynamic_pointer_cast<AddressInfoMulticast>(sda.addr);
                    if (addr == NULL) {
                        BOOST_LOG_TRIVIAL(debug) << "Replying lost " << TreeRpc::kAckF << " to " << method << " " << sda.transaction << " null addr";
                    } else {
                        BOOST_LOG_TRIVIAL(debug) << "Replying lost " << TreeRpc::kAckF << " to " << method << " " << sda.transaction << " from " << inet_ntoa(addr->addr_.sin_addr) << ":" << addr->addr_.sin_port;
                    }
                    trpc.reply(sda.uuid, sda.addr, TreeRpc::kAckF, sda.transaction, false, false, nullptr, nullptr, nullptr);
                    return;
                }
            }

            std::shared_ptr<ObserverHandle<TreeRpc::Method>> observer_ackf;

            observer_ackf = server->add(TreeRpc::kAckF, [this,&mutexf,&cond,&ackf_received,&sda](const TreeRpc::Method &method, const ServerDispatchArgs & args) -> bool {
                std::unique_lock<std::mutex> lkwait(mutexf);
                bool handled = false;
                if (method == TreeRpc::kControlForceStop) {
                    cond.notify_all();
                    return false;
                }
                try {
                    if (sda.transaction == args.transaction) {
                        handled = true;
                        ackf_received = true;
                        cond.notify_all();
                    }
                } catch(const std::runtime_error & e) {
                    std::cerr << e.what() << std::endl;
                } catch(...) {
                    BOOST_LOG_TRIVIAL(error) << "Got a unknown exception at " << basename(__FILE__) << ":" << __LINE__;
                }
                return handled;
            });

            if (method >= TreeRpc::kMethodBegin && method < TreeRpc::kMethodEnd) {
                process_method(false, server, method, sda, mutexf, cond, ackf_received);
            } else {
                switch (method) {
                case TreeRpc::kAvailable: {
                    process_avail(false, server, sda, mutexf, cond, ackf_received);
                }
                    break;
                case TreeRpc::kAlloc: {
                    process_alloc(false, server, sda, mutexf, cond, ackf_received);
                }
                    break;
                default:
                    break;
                }

            }

        }

    }
}


}
