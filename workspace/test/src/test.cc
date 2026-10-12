
#include <csignal>

#include <chrono>
#include <bitset>
#include <climits>
#include <iostream>
#include <sstream>
#include <memory>
#include <random>
#include <algorithm>
#include <thread>
#include <chrono>
#include <unordered_set>

#include <gmpxx.h>

#define BOOST_TEST_DYN_LINK 1
#include <boost/log/trivial.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/results_collector.hpp>
#include <boost/test/framework.hpp>
#include <boost/thread/thread.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include "bigdata/log_setup.hh"
#include "bigdata/veb/veb_root.hh"
#include "bigdata/globals.hh"
#include "bigdata/util.hh"
#include "bigdata/id.hh"
#include "bigdata/natural.hh"
#include "bigdata/registry.hh"
#include "bigdata/registry_unicast.hh"
#include "bigdata/factory.hh"
#include "bigdata/server_multicast.hh"
#include "bigdata/channel_multicast.hh"
#include "bigdata/tree_rpc_multicast.hh"
#include "bigdata/thread_pool.hh"
#include "bigdata/processor_node.hh"
#include "bigdata/processor_cheater.hh"
#include "bigdata/statistc.hh"
#include "bigdata/veb/veb_client.hh"

#include "bigdata/properties.hh"

static std::shared_ptr<bigdata::Factory> factory_;
static std::shared_ptr<bigdata::Registry> registry_;
static std::shared_ptr<bigdata::veb::VebRoot> root_;
static std::shared_ptr<bigdata::Server> server_;
static std::shared_ptr<bigdata::IChannel> channel_;
static std::shared_ptr<bigdata::IProcessor> processor_;


static volatile bool paused_ = false;
static volatile bool quitting_ = false;


// BOOST_CHECK( add( 2,2 ) == 4 );        // #1 continues on error
// BOOST_REQUIRE( add( 2,2 ) == 4 );      // #2 throws on error
// BOOST_ERROR( "Ouch..." );            // #3 continues on error
// BOOST_FAIL( "Ouch..." );             // #4 throws on error
// BOOST_CHECK_MESSAGE( add( 2,2 ) == 4, "add(..) result: " << add( 2,2 )); // #6 continues on error
// BOOST_CHECK_EQUAL( add( 2,2 ), 4 );	  // #7 continues on error

#if 0
static void test_uuid() {
    bigdata::tree_uuid_t uuid = 0;

    BIGDATA_SET_ROOT(uuid);
    BOOST_CHECK(uuid == __UINT64_C(0x8000000000000000));

    BIGDATA_UNSET_ROOT(uuid);
    BOOST_CHECK(uuid == 0x0ull);

    boost::unit_test::framework::master_test_suite().argc;
    boost::unit_test::framework::master_test_suite().argv;
}

static void test_natural() {
    std::stringstream ss;
    std::stringstream sc;
    bigdata::Natural natural;

    BOOST_CHECK(bigdata::log2(1u<<0) == 0);
    BOOST_CHECK(bigdata::log2(1u<<1) == 1);
    BOOST_CHECK(bigdata::log2(1u<<8) == 8);
    BOOST_CHECK(bigdata::log2(1u<<14) == 14);
    BOOST_CHECK(bigdata::log2(1u<<31) == 31);

    ss << "d";
    ss >> std::hex >> natural;
    sc << std::hex << natural;

    BOOST_CHECK("d" == sc.str());
    BOOST_CHECK(natural.bits() == 4);

    sc = std::stringstream();

    BOOST_CHECK(natural.high(natural.bits() >> 1) == 0x3);
    BOOST_CHECK(natural.low(natural.bits() >> 1) == 0x1);

    natural <<= 1;

    BOOST_CHECK(natural == 0x1a);
    BOOST_CHECK(natural.bits() == 8);

    natural >>= 2;

    BOOST_CHECK(natural == 0x6);
    BOOST_CHECK(natural.bits() == 8);

}

static void test_rpc() {
    //auto id = std::make_shared<bigdata::Id>();
    //bigdata::TreeRpcMulticast trpc(server_, root_->uuid(), id, false);

    //std::lock_guard<std::recursive_mutex> lk(channel_->mutex());
    //trpc.insert(bigdata::Natural(30), {});
    // trpc.remove(bigdata::Natural(30));

    // std::this_thread::sleep_for (std::chrono::seconds(10));
}


static void test_memory() {
    bigdata::Natural n;
    n = 1;

    for (auto i=1u, j=0u; i <= (1u << 30u); j++, i <<= 1) {
        if (quitting_) return;
        try {
            n = 1;
            n = n << (i-1);
            BOOST_CHECK_MESSAGE(i == n.bits(), "i(=" + std::to_string(i) + ") != bits(=" + std::to_string(n.bits()) + ")");
        } catch (const std::runtime_error & e) {
            // std::cout << std::endl;
            // std::cout << e.what() << std::endl;
            // std::cout << '(' << j << ", ";
            // std::cout << bigdata::format_with_commas(i) << ", ";
            // std::cout << bigdata::format_with_commas(n.bits()) << ')';
            BOOST_ERROR(e.what());
            break;
        }

        // std::cout << std::hex << n << ", ";
        // std::cout << '(' << j << ", ";
        // std::cout << bigdata::format_with_commas(i) << ", ";
        // std::cout << bigdata::format_with_commas(n.bits()) << ')';
        // std::cout << bigdata::format_with_commas(bigdata::get_used_memory()) << ", ";
    }
    // std::cout << std::endl;
    // std::cout << "Memory used by process: " << bigdata::format_with_commas(bigdata::get_used_memory()) << std::endl;
    BOOST_CHECK(bigdata::get_proc_memory() > n.bits()/8ull);
}

static void test_ids() {
    bigdata::Id id1;
}
#endif


template <class T>
static void test_tree_correctness(std::vector<T> & tdata) {
    int32_t i=0;
    const int32_t print_each = 100;

    BOOST_LOG_TRIVIAL(info) << "Correctness test";

    for (auto v : tdata) {
        if (quitting_) return;
        if ((i++ % print_each) == print_each - 1) {
            std::cout << std::endl;
            BOOST_LOG_TRIVIAL(info) << "inserting " << i << '/' << tdata.size();
        } else {
            std::cout << '.' << std::flush;
        }
        auto h = std::get<0>(root_->insert(bigdata::Natural(v), {}));
        BOOST_CHECK_MESSAGE(h == false, v.to_string() + " (insert)found");
    }
    std::cout << std::endl;

    std::sort(tdata.begin(), tdata.end());

    BOOST_LOG_TRIVIAL(info) << "search";

    for (auto v=0u; v < tdata.size(); v++) {
        if (quitting_) return;
        if (v % print_each == print_each - 1) {
            std::cout << std::endl;
            BOOST_LOG_TRIVIAL(info) << "search " << v << '/' << tdata.size();
        } else {
            std::cout << '.' << std::flush;
        }
        auto k = root_->search(bigdata::Natural(tdata[v])).first;
        BOOST_CHECK_MESSAGE(k == true, tdata[v].to_string() + " not found");
    }
    std::cout << std::endl;

    BOOST_CHECK_MESSAGE(root_->min().first == tdata[0], root_->min().first.to_string() + " (min)!=" + tdata[0].to_string());
    BOOST_CHECK_MESSAGE(root_->max().first == tdata[tdata.size()-1], root_->max().first.to_string() + " (max)!=" + tdata[tdata.size()-1].to_string());

    BOOST_LOG_TRIVIAL(info) << "predecessor";

    for (auto v=1u; v < tdata.size(); v++) {
        if (quitting_) return;
        if (v % print_each == print_each - 1) {
            std::cout << std::endl;
            BOOST_LOG_TRIVIAL(info) << "predecessor " << v << '/' << tdata.size();
        } else {
            std::cout << '.' << std::flush;
        }
        bigdata::Natural p = root_->predecessor(bigdata::Natural(tdata[v])).first;
        BOOST_CHECK_MESSAGE(p == tdata[v-1], p.to_string() + " (predecessor)!= " + tdata[v-1].to_string());
    }
    std::cout << std::endl;

    BOOST_LOG_TRIVIAL(info) << "successor";

    for (auto v=0u; v < tdata.size()-1; v++) {
        if (quitting_) return;
        if (v % print_each == print_each - 1) {
            std::cout << std::endl;
            BOOST_LOG_TRIVIAL(info) << "successor " << v << '/' << tdata.size();
        } else {
            std::cout << '.' << std::flush;
        }
        bigdata::Natural s = root_->successor(bigdata::Natural(tdata[v])).first;
        BOOST_CHECK_MESSAGE(s == tdata[v+1], s.to_string() + " (successor)!= " + tdata[v+1].to_string());
    }
    std::cout << std::endl;

    BOOST_LOG_TRIVIAL(info) << "deleting/searching";

    std::random_shuffle(tdata.begin(), tdata.end());

    for (auto v=0u; v < tdata.size(); v++) {
        if (quitting_) return;
        if (v % print_each == print_each - 1) {
            std::cout << std::endl;
            BOOST_LOG_TRIVIAL(info) << "deleting/searching " << v << '/' << tdata.size();
        } else {
            std::cout << '.' << std::flush;
        }
        root_->remove(bigdata::Natural(tdata[v]));
        auto k = root_->search(bigdata::Natural(tdata[v])).first;
        if (k) {
            root_->remove(bigdata::Natural(tdata[v]));
            k = root_->search(bigdata::Natural(tdata[v])).first;
        }
        BOOST_CHECK_MESSAGE(k == false, tdata[v].to_string() + " found");
    }
    std::cout << std::endl;

    BOOST_CHECK_MESSAGE(root_->min().first.is_valid() == false, root_->min().first.to_string() + " (min) not empty");
    BOOST_CHECK_MESSAGE(root_->max().first.is_valid() == false, root_->max().first.to_string() + " (max) not empty");
}

static void test_tree_performance(bigdata::TreeRpc::Method method, const std::unordered_set<bigdata::Natural> & keys, int32_t index, const int32_t num_collect_statistic) {
    const int print_header_each = 100;

    BOOST_LOG_TRIVIAL(info) << "Performance test";

    auto it = std::next(keys.begin(), index);

    for(auto count=0; count < num_collect_statistic && it != keys.end(); it++, count++) {
        while(paused_) {
            if (quitting_) return;
            usleep(100 * 1000);
        }
        if (quitting_) return;

        if (std::distance(keys.begin(), it) % print_header_each == 0) {
            std::cout << std::endl;
            BOOST_LOG_TRIVIAL(info) << method << " - " << std::distance(keys.begin(), it) + 1 << "/" << keys.size();
        } else {
            std::cout << '.' << std::flush;
        }
        BOOST_LOG_TRIVIAL(trace) << "Key under test (" << method << ") " << *it;

        switch (method) {
        case bigdata::TreeRpc::kInsert:
            root_->insert(*it, {});
            break;
        case bigdata::TreeRpc::kSearch:
            root_->search(*it);
            break;
        case bigdata::TreeRpc::kSuccessor:
            root_->successor(*it);
            break;
        case bigdata::TreeRpc::kPredecessor:
            root_->predecessor(*it);
            break;
        case bigdata::TreeRpc::kRemove:
            root_->remove(*it);
            break;
        default:
            throw std::runtime_error("Not testing method " + bigdata::TreeRpc::to_string(method));
        }
    }

    std::cout << std::endl;
    BOOST_LOG_TRIVIAL(info) << method << " From " << index + 1 << " to " << std::distance(keys.begin(), it);
}

static void test_registry_performance() {
    bigdata::Natural::Seed seed;

    std::unordered_set<bigdata::Natural> registry;

    for(auto iter=1; iter <= 100; iter++) {

        for(auto level=17u; level <=17; level++) {
            std::vector<bigdata::Natural> keys;

            auto universe = 1 << level;

            for(auto c=0u; c < 2000; c++) {
                if (quitting_) return;
                bigdata::Natural k(1);
                k = k << (universe - 1);
                k = k | bigdata::Natural(1);
                k = k | bigdata::Natural::random(seed, universe);
                keys.push_back(k);
                registry.insert(k);
            }

            auto start = std::chrono::steady_clock::now();
            auto aux_universe = 1;

            {

                auto sample = bigdata::statistic::include_thread_sample();

                bigdata::statistic::thread_sample_set_level(universe);

                {
                    auto time_couter = std::shared_ptr<void>(NULL, [&start,&sample, aux_universe, universe](void*){
                        if (sample != NULL && false == std::uncaught_exception()) {
                            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

                            sample->normalize_level(aux_universe);
                            sample->duration = duration.count();
                            std::cout << " universe " << universe << " duration " << duration.count() << std::endl;
                            bigdata::statistic::Statistic::add(*sample, bigdata::TreeRpc::kVeb_expanded);
                        }
                    });

                    for(auto & key : keys) {
                        auto k = registry.find(key);
                    }

                }

            }
        }

        // std::cout << bigdata::statistic::Statistic::get();
        bigdata::statistic::Statistic::get().save(iter);
        bigdata::statistic::Statistic::zero();

    }
}

static void test_natural_performance() {
    bigdata::Natural::Seed seed;

    for(auto iter=1; iter <= 33; iter++) {

        for(auto level=1u; level <=17; level++) {
            std::vector<bigdata::Natural> keys;

            auto universe = 1 << level;

            for(auto c=0u; c < 2000; c++) {
                if (quitting_) return;
                bigdata::Natural k(1);
                k = k << (universe - 1);
                k = k | bigdata::Natural(1);
                k = k | bigdata::Natural::random(seed, universe);
                keys.push_back(k);
            }

            auto start = std::chrono::steady_clock::now();
            auto aux_universe = 1;

            {

                auto sample = bigdata::statistic::include_thread_sample();

                bigdata::statistic::thread_sample_set_level(universe);

                {
                    auto time_couter = std::shared_ptr<void>(NULL, [&start,&sample, aux_universe, universe](void*){
                        if (sample != NULL && false == std::uncaught_exception()) {
                            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

                            sample->normalize_level(aux_universe);
                            sample->duration = duration.count();
                            std::cout << " universe " << universe << " duration " << duration.count() << std::endl;
                            bigdata::statistic::Statistic::add(*sample, bigdata::TreeRpc::kVeb_expanded);
                        }
                    });

                    for(auto & key : keys) {
                        auto high = key.high(universe >> 1);
                        auto low = key.low(universe >> 1);
                        auto val = (high << (universe >> 1)) | low;
                    }

                }

            }
        }

        // std::cout << bigdata::statistic::Statistic::get();
        bigdata::statistic::Statistic::get().save(iter);
        bigdata::statistic::Statistic::zero();

    }

}

static void test_tree_correctness_big_no_check() {
    bigdata::statistic::Statistic::include();
    std::unordered_set<bigdata::Natural> keys;
    const int32_t numinsert = 65535; // UINT64_MAX;
    const int32_t num_collect_statistic = 2000;
    BOOST_LOG_TRIVIAL(debug) << "test_tree_correctness_big_no_check";

    bigdata::Natural::Seed seed;
    int32_t i = 0;

    try {
        while(keys.size() < numinsert) {
            if (quitting_) return;
            keys.insert(bigdata::Natural::random(seed, bigdata::Properties::get_instance()->test_maxbits()));
        }

        if (bigdata::Properties::get_instance()->test_mode() == "registry") {
            test_registry_performance();
            return;
        }


        if (bigdata::Properties::get_instance()->test_mode() == "natural") {
            test_natural_performance();
            return;
        }

        BOOST_LOG_TRIVIAL(info) << "preparing input data...";

        if (bigdata::Properties::get_instance()->test_mode() == "correctness") {
            std::vector<bigdata::Natural> correctness(keys.size());
            std::copy(keys.begin(), keys.end(), correctness.begin());
            BOOST_LOG_TRIVIAL(info) << "input data ready";
            test_tree_correctness(correctness);
            return;
        }

        BOOST_LOG_TRIVIAL(info) << "input data ready";

        for(int32_t i=0, s=0; i < static_cast<int32_t>(keys.size()); i += num_collect_statistic, s++) {
            int32_t n = num_collect_statistic;
            if (n > static_cast<int32_t>(keys.size()) - i) {
                n = static_cast<int32_t>(keys.size()) - i;
            }
            if (n > 0) {
                test_tree_performance(bigdata::TreeRpc::kInsert, keys, i, n);
                test_tree_performance(bigdata::TreeRpc::kSuccessor, keys, i, n);
                test_tree_performance(bigdata::TreeRpc::kPredecessor, keys, i, n);
                test_tree_performance(bigdata::TreeRpc::kSearch, keys, i, n);

                std::cout << bigdata::statistic::Statistic::get();
                bigdata::statistic::Statistic::get().save(s+1);
                bigdata::statistic::Statistic::zero();
            }
        }

        for(int32_t i=0, s=0; i < static_cast<int32_t>(keys.size()); i += num_collect_statistic, s++) {
            int32_t n = num_collect_statistic;
            if (n > static_cast<int32_t>(keys.size()) - i) {
                n = static_cast<int32_t>(keys.size()) - i;
            }
            if (n > 0) {
                test_tree_performance(bigdata::TreeRpc::kRemove, keys, i, num_collect_statistic);

                std::cout << bigdata::statistic::Statistic::get();
                bigdata::statistic::Statistic::get().save(s+1);
                bigdata::statistic::Statistic::zero();
            }
        }

    } catch (const std::runtime_error & e) {
        BOOST_LOG_TRIVIAL(fatal) << "Except with i = " << i << ". " << e.what();
        throw e;
    } catch (...) {
        BOOST_LOG_TRIVIAL(fatal) << "Except with i = " << i << ".";
        throw;
    }
}


static bool init_test_suite() {
    // boost::unit_test::test_suite* ts_bigdata_correctness = BOOST_TEST_SUITE( "ts_bigdata_correctness" );
    //ts_bigdata_correctness->add( BOOST_TEST_CASE( &test_uuid ) );
    //ts_bigdata_correctness->add( BOOST_TEST_CASE( &test_natural ) );
    //ts_bigdata_correctness->add( BOOST_TEST_CASE( &test_memory ) );
    //ts_bigdata_correctness->add( BOOST_TEST_CASE( &test_ids ) );
    //ts_bigdata_correctness->add( BOOST_TEST_CASE( &test_rpc ) );

    boost::unit_test::test_suite* ts_tree_correctness = BOOST_TEST_SUITE( "ts_tree_correctness" );
    //ts_tree_correctness->add( BOOST_TEST_CASE( &test_tree_correctness ) );
    //ts_tree_correctness->add( BOOST_TEST_CASE( &test_tree_correctness_big ) );
    ts_tree_correctness->add( BOOST_TEST_CASE( &test_tree_correctness_big_no_check ) );

    //boost::unit_test::framework::master_test_suite().add( ts_bigdata_correctness );
    boost::unit_test::framework::master_test_suite().add( ts_tree_correctness );

    // boost::unit_test::unit_test_log.set_threshold_level( boost::unit_test::log_level::log_successful_tests );

    return true;
}

static void signal_handler(int signal)
{
    if (signal == SIGINT) {
        quitting_ = true;
        if (server_) {
            server_->stop();
        }
    } else if (signal == SIGUSR1) {
        paused_ = !paused_;
    }
}


int main(int argc, char *argv[]) {
    int64_t duration = 0;
    BOOST_LOG_SCOPED_THREAD_TAG("ThreadID", boost::this_thread::get_id());
    int ret_val = -1;

    if (argc > 1) {
        if (std::string("help") == argv[1]) {
            bigdata::Properties::get_instance()->print_help(std::cout);
            return 0;
        }
        try {
            bigdata::Properties::get_instance()->parse(std::cin);
        } catch (const std::runtime_error & e) {
            BOOST_LOG_TRIVIAL(warning) << "Property parse failed: " << e.what();
            throw e;
        } catch (...) {
            BOOST_LOG_TRIVIAL(warning) << "Property parse failed";
            throw;
        }
        bigdata::Properties::get_instance()->validate();
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGUSR1, signal_handler);

    bigdata::log::setup(bigdata::Properties::get_instance()->log_level());

    BOOST_LOG_TRIVIAL(info) << "Tree uuid: " << bigdata::Properties::get_instance()->root_uuid();
    BOOST_LOG_TRIVIAL(info) << "Port: " << bigdata::Properties::get_instance()->port();
    BOOST_LOG_TRIVIAL(info) << "System Memory: " << bigdata::format_with_commas(bigdata::get_total_memory());
    BOOST_LOG_TRIVIAL(info) << "Available Memory: " << bigdata::format_with_commas(bigdata::get_available_memory());
    BOOST_LOG_TRIVIAL(info) << "Role: " << bigdata::Properties::get_instance()->role();

    if (bigdata::Properties::get_instance()->memory_threshold() < 15) {
        BOOST_LOG_TRIVIAL(warning) << "Memory threshold is " << static_cast<int32_t>(bigdata::Properties::get_instance()->memory_threshold()) << "%. At least 15% is recommended.";
    }

    channel_ = std::make_shared<bigdata::ChannelMulticast>();
    registry_ = std::make_shared< bigdata::RegistryUnicast > ();
    factory_ = std::make_shared<bigdata::Factory>();

    if (bigdata::Properties::get_instance()->role() == "root") {
        processor_ = std::make_shared<bigdata::ProcessorNode>(registry_, factory_);
    } else if (bigdata::Properties::get_instance()->role() == "client") {
        processor_ = std::make_shared<bigdata::ProcessorNode>(registry_, factory_);
    } else if (bigdata::Properties::get_instance()->role() == "node") {
        processor_ = std::make_shared<bigdata::ProcessorNode>(registry_, factory_);
    } else if (bigdata::Properties::get_instance()->role() == "cheater") {
        processor_ = std::make_shared<bigdata::ProcessorCheater>(registry_, factory_);
    }

    server_ = std::make_shared< bigdata::ServerMulticast > (registry_, factory_, processor_);

    if (bigdata::Properties::get_instance()->role() == "root") {
        root_ = std::make_shared<bigdata::veb::VebRoot>(bigdata::Properties::get_instance()->root_uuid(), server_, factory_, registry_);
        registry_->sign(bigdata::Properties::get_instance()->root_uuid(), root_);
    } else if (bigdata::Properties::get_instance()->role() == "client") {
        throw std::runtime_error("Still not implemented");
        // TODO: Ajustar Client p/ flag to_root = true
        // bigdata::VebClient(1, std::shared_ptr<const Id> id, std::shared_ptr<TreeRpc> tree_rpc);
    }

    BOOST_LOG_TRIVIAL(debug) << "Starting server.";

    server_->start();

    if (bigdata::Properties::get_instance()->test_run()) {
        auto start = std::chrono::steady_clock::now();
        auto time_couter = std::shared_ptr<void>(NULL, [&start,&duration](void*){
            duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
        });

        BOOST_LOG_TRIVIAL(debug) << "Starting test.";
        try {
            boost::unit_test::unit_test_main(init_test_suite, argc, argv);
            const boost::unit_test::test_results& theResults = boost::unit_test::results_collector.results( boost::unit_test::framework::master_test_suite().p_id );
            ret_val = theResults.passed() ? 0 : -1;
        } catch (const std::runtime_error & e) {
            BOOST_LOG_TRIVIAL(error) << "unit_test_main() finished with an execption";
            BOOST_LOG_TRIVIAL(error) << e.what();
            server_->stop();
        } catch (...) {
            BOOST_LOG_TRIVIAL(fatal) << "Test returned unknown exception";
            server_->stop();
        }
    } else {
        ret_val = 0;
    }

    //BOOST_LOG_TRIVIAL(debug) << "Stoping server.";

    //server_->stop();

    if (bigdata::Properties::get_instance()->test_run()) {
        BOOST_LOG_TRIVIAL(info) << "Printing statistics.";
        try {
            // BOOST_LOG_TRIVIAL(info) << bigdata::statistic::Statistic::get();
        } catch (...) {

        }
        BOOST_LOG_TRIVIAL(info) << "Experiment finished in " << bigdata::format_with_commas(duration) << " us";
        system("mkdir -p statistics");
        system("rm -f statistics/*");
        bigdata::statistic::Statistic::save("statistics/");
    }

    BOOST_LOG_TRIVIAL(info) << "Joining server.";

    if (bigdata::Properties::get_instance()->service() == false) {
        server_->stop();
    }

    server_->join();

    BOOST_LOG_TRIVIAL(info) << "About to finish.";

    BOOST_LOG_TRIVIAL(info) << "Available Memory: " << bigdata::format_with_commas(bigdata::get_available_memory());

    return ret_val;
}

// colocar no overleaf que as coisas deveria entrar como DLL dinamica para ser atualizadas em tempo de execução
// implementação mista com btree
// ifconfig eth1 mtu 9000 up
// vi /etc/network/interfaces
// mtu 9000
// /etc/init.d/networking restart


// consigo fazer a VandeBoas trabalhar com chaves como string. com tamanho fixo eh trivial. com tamanho variavel, ao crescer a arvore, bastaria shiftar todos os nos p cima recursivamente
// mas então p fazer isso, p não precisar ajustar todos os nós, bataria passar p baixo qual o novo universo da arvore quando chamar o método dela.

