//============================================================================
// Name        : DvEB.cpp
// Author      : Edgard Lima
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <cstdio>
#include <ctime>

#include <signal.h>

#include <iostream>
#include <memory>
#include <chrono>

#include "dveb/Util.hh"

#include <boost/program_options.hpp>

#include "dveb/TestIntro3.hh"
#include "dveb/ProtoVEBClient.hh"
#include "dveb/ProtoVEBk.hh"
#include "dveb/DynamicProtoVEB.hh"

#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>

// k | u                    | max | sqrt(u)    | log(u) |
// ------------------------------------------------------
// 0 | 2                    | u-1 |            | 0      |
// 1 | 4                    | u-1 | 2          | 2      |
// 2 | 16                   | u-1 | 4          | 4      |
// 3 | 256                  | u-1 | 16         | 8      |
// 4 | 65536                | u-1 | 256        | 16     |
// 5 | 4294967296           | u-1 | 65536      | 32     |
// 6 | 18446744073709551616 | u-1 | 4294967296 | 64     |

#if 0
static void print_present(ProtoVEB & pvEB, bool detail=true) {
  std::pair<uint64_t, std::shared_ptr<ProtoData>> res;

  std::cout << "present {";

  for(res = pvEB.getMin(); NULL != res.second; res = pvEB.getSuccessor(res.first)) {
    if (detail) {
      ProtoDataString *pdStr = dynamic_cast<ProtoDataString *>(res.second.get());
      std::cout << res.first << "('"<< pdStr->str_ << "') ";
    } else {
      std::cout << res.first << " ";
    }
  }

  std::cout << "}" << std::endl;
}

static void print_sucessor(ProtoVEB & pvEB, uint64_t x) {
  std::shared_ptr<ProtoDataString> pdStr;
  std::shared_ptr<ProtoData> pdStrAux;

  std::tie(std::ignore, pdStrAux) = pvEB.getSuccessor(x);
  pdStr = std::dynamic_pointer_cast<ProtoDataString>(pdStrAux);
  if (NULL != pdStr) {
    std::cout << "Successor of " << x << " is '" << pdStr->str_ << "'." << std::endl;
  } else {
    std::cout << x << " has no successor." << std::endl;
  }
}
#endif

void sighandler(const boost::system::error_code& error, int signal_number) {
  if (!error) {
    // A signal occurred.
  }
}

static void console() {

  // Construct a signal set registered for process termination.
  std::shared_ptr<boost::asio::io_service> io_service(new boost::asio::io_service());
  boost::asio::signal_set signals(*io_service, SIGINT, SIGTERM);

  // Start an asynchronous wait for one of the signals to occur.
  signals.async_wait(sighandler);

  std::shared_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(*io_service));
  boost::thread t(static_cast<std::size_t (boost::asio::io_service::*)()>(&boost::asio::io_service::run), io_service);

  std::cout << "Console started. h-help. c-cancel. s=status" << std::endl;
  std::cout << "choose an option (and hit ENTER):" << std::endl;

  try {

    while (false == test_cancel_) {

      int ch = ::getchar();

      if (ch == 'h') {
        std::cout << "Available options. h-help. c-cancel. s=status" << std::endl;
      } else if (ch == 'c') {
        test_cancel_ = true;
        break;
      } else if (ch == 's') {
        test_status_ = true;
      } else if (ch == EOF) {

      } else if (ch == '\n') {

      } else {
        std::cout << "Available options. h-help. c-cancel. s=status" << std::endl;
      }
    }

  } catch (...) {

  }

  work.reset();
  t.join();

  std::cout << "Console finished" << std::endl;
}

static void test_remote_simple(uint16_t remote_port, const std::string & test_mode, uint64_t clusters,
    const std::string & tree_type, uint16_t universe, uint64_t loop_first, uint64_t loop_last, int64_t loop,
    uint16_t verbose, std::string & generator) {
  uint64_t used_mem_start = getUsedMemory();

  std::cout << "memory start: " << FormatWithCommas(used_mem_start) << std::endl;

  std::shared_ptr<ProtoVEB> pVEBC;
  if (0 == tree_type.compare("client")) {
    pVEBC.reset(new ProtoVEBClient("127.0.0.1", std::to_string(remote_port)));
  } else if (0 == tree_type.compare("dynamic")) {
    pVEBC.reset(new DynamicProtoVEB(universe));
  } else if (0 == tree_type.compare("k")) {
    pVEBC = CreatePVEBFromK(universe);
  } else {
    std::runtime_error e(std::string("unknown tree-type: " + tree_type));
    throw e;
  }

  std::cout << "Tree created: " << FormatWithCommas(getUsedMemory()) << std::endl;
  std::cout << "Process memory in bytes: " << FormatWithCommas(getUsedMemory()) << std::endl;

  /*

   try {
   uint64_t x;
   std::shared_ptr<ProtoData> pd;
   std::shared_ptr<ProtoDataString> pds;

   std::tie(x, pd) = pVEBC.getMin();
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Min element is " << x << ", " << pds->str_ << std::endl;
   } else {
   std::cout << "there is no Min element." << std::endl;
   }

   std::tie(x, pd) = pVEBC.getMax();
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Max element is " << x << ", " << pds->str_ << std::endl;
   } else {
   std::cout << "there is no Max element." << std::endl;
   }

   pds.reset(new ProtoDataString("Ola eu sou 10000"));
   pd = pVEBC.insert(10000, pds);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Element replaced by 10000 was " << pds->str_ << std::endl;
   } else {
   std::cout << "There was no element data in 10000." << std::endl;
   }

   pd = pVEBC.get(10000);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << pds->str_ << std::endl;
   } else {
   std::cout << "there is no element 10000." << std::endl;
   }

   std::tie(x, pd) = pVEBC.getMin();
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Min element is " << x << ", " << pds->str_ << std::endl;
   } else {
   std::cout << "there is no Min element." << std::endl;
   }

   std::tie(x, pd) = pVEBC.getMax();
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Max element is " << x << ", " << pds->str_ << std::endl;
   } else {
   std::cout << "there is no Max element." << std::endl;
   }


   std::tie(x, pd) = pVEBC.getSuccessor(9000);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Successor of 9000 is " << x << ", " << pds->str_ << std::endl;
   } else {
   std::cout << "there is no Successor of 9000." << std::endl;
   }

   std::tie(x, pd) = pVEBC.getSuccessor(10000);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Successor of 10000 is " << x << ", " << pds->str_ << std::endl;
   } else {
   std::cout << "there is no Successor of 10000." << std::endl;
   }

   pd = pVEBC.remove(10000);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Element removed from 10000 was " << pds->str_ << std::endl;
   } else {
   std::cout << "There was no element data in 10000." << std::endl;
   }

   pd = pVEBC.remove(10000);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Element removed from 10000 was " << pds->str_ << std::endl;
   } else {
   std::cout << "There was no element data in 10000." << std::endl;
   }

   pds.reset(new ProtoDataString("Hello I'm the new 10000"));
   pd = pVEBC.insert(10000, pds);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Element rep[laced by 10000 was " << pds->str_ << std::endl;
   } else {
   std::cout << "There was no element data in 10000." << std::endl;
   }

   pds.reset(new ProtoDataString("Hello I'm the newest 10000"));
   pd = pVEBC.insert(10000, pds);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Element replaced by 10000 was " << pds->str_ << std::endl;
   } else {
   std::cout << "There was no element data in 10000." << std::endl;
   }

   std::tie(x, pd) = pVEBC.getPredecessor(9000);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Predecessor of 9000 is " << x << ", " << pds->str_ << std::endl;
   } else {
   std::cout << "there is no Predecessor of 9000." << std::endl;
   }

   std::tie(x, pd) = pVEBC.getPredecessor(11000);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Predecessor of 11000 is " << x << ", " << pds->str_ << std::endl;
   } else {
   std::cout << "there is no Predecessor of 11000." << std::endl;
   }

   pd = pVEBC.remove(10000);
   pds = std::dynamic_pointer_cast<ProtoDataString>(pd);
   if (NULL != pds) {
   std::cout << "Element removed from 10000 was " << pds->str_ << std::endl;
   } else {
   std::cout << "There was no element data in 10000." << std::endl;
   }

   } catch (std::exception& e) {
   std::cerr << "Exception in thread: " << e.what() << std::endl;
   }

   */

  boost::thread t(boost::bind(console));

  if (0 == test_mode.compare("test")) {
    test_tree(*pVEBC, used_mem_start, clusters, loop);
  } else if (0 == test_mode.compare("performance")) {
	  test_tree_performance_n_to_N_delete_end(*pVEBC, used_mem_start, clusters, loop_first, loop_last, loop, false, verbose, generator);
  } else if (0 == test_mode.compare("insert_only")) {
	  test_tree_performance_n_to_N_delete_end(*pVEBC, used_mem_start, clusters, loop_first, loop_last, loop, true, verbose, generator);
  } else if (0 == test_mode.compare("dump")) {
    print_tree(*pVEBC, used_mem_start);
  } else if (0 == test_mode.compare("erase_all")) {
    uint64_t x;
    std::shared_ptr<ProtoData> pd;
    while (true) {
      std::tie(x, pd) = pVEBC->getMin();
      if (pd != NULL) {
        pVEBC->remove(x);
      } else {
        break;
      }
    }
  } else {
    std::runtime_error e(std::string("unknown test-mode: " + test_mode));
    throw e;
  }

  std::cout << "About to finish..." << std::endl;

  test_cancel_ = true;

  // todo: need to close console thread here
  pthread_kill(t.native_handle(), SIGTERM);
  t.join();

  return;
}

int main(int argc, char * argv[]) {
  time_t time_tt;

  // return test_intro();ProtoDataString
  //for (uint64_t k=2; k < 64; k++) {
  //  test_intro3(k);
  //}

  uint16_t verbose=0;
  uint16_t remote_port=8083;
  std::string test_mode;
  uint64_t clusters=0;
  std::string tree_type;
  uint16_t universe=0;
  uint16_t loop_first=0;
  uint16_t loop_last=0;
  int64_t loop=0;
  std::string generator;

  namespace bpo = boost::program_options;
  bpo::options_description desc("Options");
  desc.add_options()
      ("help,h", "Print help message")
      ("verbose,v", bpo::value<uint16_t>()->default_value(0), "Print more verbose messages")
      ("remote-port,r", bpo::value<uint16_t>()->default_value(8083), "Remote tree port (only applies to 'client' tree type)")
      ("test-mode,m", bpo::value<std::string>()->required(), "Test mode (dump,test,print,performance,insert_only,erase_all)")
      ("clusters,c", bpo::value<uint64_t>()->default_value(2), "Number of clusters (used to calculate max key values generated during tests)")
      ("type,t",bpo::value<std::string>()->default_value("k"), "Tree type (client,k,dynamic)")
      ("universe,u", bpo::value<uint16_t>()->default_value(32), "Tree universe (2 power this number)")
      ("loop-first,f", bpo::value<uint16_t>()->default_value(1), "How many keys to add on first interaction (2 power this number)")
      ("loop-last,l", bpo::value<uint16_t>()->default_value(10), "How many keys to add on last interaction (2 power this number)")
      ("loop,o", bpo::value<int64_t>()->default_value(1), "How many times repeat the test (use negative value to loop forever)")
      ("generator,g", bpo::value<std::string>()->default_value("uniform"), "Keys generator (uniform,cluster_seq)");

  bpo::variables_map vm;

  try {
    bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
    bpo::notify(vm);
  } catch (std::exception & e) {
    std::cerr << e.what() << std::endl;
    std::cout << desc << std::endl;
    return -1;
  }

  if (vm.count("help") > 0) {
    std::cout << desc << std::endl;
    return 1;
  }

  if (vm.count("verbose")) {
    verbose = vm["verbose"].as<uint16_t>();
  }
  if (vm.count("remote-port")) {
    remote_port = vm["remote-port"].as<uint16_t>();
  }
  if (vm.count("test-mode")) {
    test_mode = vm["test-mode"].as<std::string>();
    std::transform(test_mode.begin(), test_mode.end(), test_mode.begin(), ::tolower);
  }
  if (vm.count("clusters")) {
    clusters = vm["clusters"].as<uint64_t>();
  }
  if (vm.count("type")) {
    tree_type = vm["type"].as<std::string>();
    std::transform(tree_type.begin(), tree_type.end(), tree_type.begin(), ::tolower);
  }
  if (vm.count("universe")) {
    universe = vm["universe"].as<uint16_t>();
  }
  if (vm.count("loop-first")) {
    loop_first = vm["loop-first"].as<uint16_t>();
  }
  if (vm.count("loop-last")) {
    loop_last = vm["loop-last"].as<uint16_t>();
  }
  if (vm.count("loop")) {
    loop = vm["loop"].as<int64_t>();
  }
  if (vm.count("generator")) {
    generator = vm["generator"].as<std::string>();
    std::transform(generator.begin(), generator.end(), generator.begin(), ::tolower);
  }

  time_tt = ::time(NULL);
  std::cout << "Started at " << ::asctime(::localtime(&time_tt)) << std::endl;
  auto start_ts = std::chrono::steady_clock::now();

  std::cout << "using: remote-port=" << remote_port << ", test-mode=" << test_mode << ", clusters=" << clusters
      << ", type=" << tree_type << ", universe=" << (int) universe << ", loop-first=" << loop_first << ", loop-last="
      << loop_last << ", loop=" << loop << ", generator=" << generator  << std::endl;

  std::cout << "Process memory in bytes: " << FormatWithCommas(getUsedMemory()) << std::endl;

  test_remote_simple(remote_port, test_mode, clusters, tree_type, universe, loop_first, loop_last, loop, verbose, generator);

  auto diff = std::chrono::steady_clock::now() - start_ts;
  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);

  time_tt = ::time(NULL);
  std::cout << "Finished at " << ::asctime(::localtime(&time_tt));
  std::cout << "Elapsed " << FormatWithCommas(elapsed_ms.count()) << " ms." << std::endl;

  return 0;
}
