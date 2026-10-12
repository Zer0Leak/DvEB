
#include <stdexcept>
#include <iomanip>

#include "boost/property_tree/info_parser.hpp"

#include "bigdata/properties.hh"

namespace bigdata {

static const uint16_t kPort = 34343;
static const char kMulticastGroup[] = "225.0.0.37";
static const char kRole[] = "root";  // root | client | node | cheater
static const int32_t kTimeout = 2;
static const int32_t kJumpMulti = 12;
static const int32_t kRetries = 20;
static const uint32_t kEnoughServersAvailable = 1;
static const uint32_t kMemoryThreshold = 0;
static const uint32_t kUdpBufSize = 16384 * 2 + 1024; // kMaxNaturalBits/8 * 2 + 1024
static const uint16_t kThreadPollSize = 4;
static const boost::log::trivial::severity_level kLogLevel = boost::log::trivial::info;
static const uint32_t kRootUuid = 0;
static const bool kForceMaxSize = true;
static const bool kStatisticsSummaryOnly = false;
static const bool kMulticastLoopback = true;
static const bool kNoHosting = false;
static const bool kSelfCheating = true;
static const bool kTestRun = true;
static const uint32_t kTestMaxBits = 131072u;  // 2^17 bits = 2^14 bytes
static const char kTestMode[] = "registry";  // performance | correctness | natural | registry
static const bool kHasCheater = false;
static const bool kService = false;

std::shared_ptr<Properties> Properties::singletoon_;
std::mutex Properties::mutex_;

std::shared_ptr<Properties> Properties::get_instance() {
    std::lock_guard<std::mutex> lk(mutex_);
    if (singletoon_ == NULL) {
        singletoon_.reset(new Properties());
    }
    return singletoon_;
}

Properties::Properties()
    : port_(kPort)
    , multicast_group_(kMulticastGroup)
    , role_(kRole)
    , timeout_(kTimeout)
    , jump_multi_(kJumpMulti)
    , retries_(kRetries)
    , enough_servers_available_(kEnoughServersAvailable)
    , memory_threshold_(kMemoryThreshold)
    , udp_buf_size_(kUdpBufSize)
    , thread_poll_size_(kThreadPollSize)
    , log_level_(kLogLevel)
    , root_uuid_(kRootUuid)
    , force_maxsize_(kForceMaxSize)
    , statistics_summary_only_(kStatisticsSummaryOnly)
    , multicast_loopback_(kMulticastLoopback)
    , no_hosting_(kNoHosting)
    , self_cheating_(kSelfCheating)
    , test_run_(kTestRun)
    , test_maxbits_(kTestMaxBits)
    , test_mode_(kTestMode)
    , has_cheater_(kHasCheater)
    , service_(kService)
{
}

void Properties::validate() {
    if (role_ != "root" && role_ != "client" && role_ != "node" && role_ != "cheater") {
        throw std::runtime_error(std::string("'role' must be \"root\" | \"client\" | \"node\" | \"cheater\". or empty for default value of ") + kRole);
    }

    if (role_ == "client" && no_hosting_ == false) {
        no_hosting_ = true;
        std::cerr << "Warning 'role' is \"client\", 'no_hosting' forced to true" << std::endl;
    }

    if (role_ == "node" && no_hosting_ == true) {
        no_hosting_ = false;
        std::cerr << "Warning: 'role' is \"node\", 'no_hosting' forced to false" << std::endl;
    }

    if (self_cheating_) {
        std::cerr << "Warning: 'self_cheating' is on" << std::endl;
    }

    if (multicast_loopback_ == false && self_cheating_ == true) {
        throw std::runtime_error("'multicast_loopback' == false && 'self_cheating' == true");
    }

    if (multicast_loopback_) {
        std::cerr << "WARNING: Loopback doesn't use UDP checksum! Is it required ???" << std::endl;
    }

    if (role_ != "root" && role_ != "client") {
        if (test_run_ == true) {
            test_run_ = false;
            std::cerr << "Warning: only 'role' \"root\" or \"client\" can run test, 'run_test' forced to false" << std::endl;
        }
    } else {
        if (test_run_ == false) {
            std::cerr << "Doesn't make sense a 'role' of \"root\" or \"client\" without running test" << std::endl;
        }
    }

    if (test_maxbits_ == 0) {
        throw std::runtime_error("'test_maxbits' invalid value of " + std::to_string(test_maxbits_));
    } else if (test_maxbits_ > 131072u) {
        throw std::runtime_error("'test_maxbits'=" + std::to_string(test_maxbits_) + ". Can't be greater than 131072.");
    }

    if (test_mode_ != "performance" && test_mode_ != "correctness" && test_mode_ != "natural" && test_mode_ != "registry") {
        throw std::runtime_error("'test_mode' invalid " + test_mode_ + ". Must be \"performance\" or \"correctness\" or \"natural\" or \"registry\"");
    }

    if (udp_buf_size_ < (test_maxbits_/8) * 2 + 1024) {
        throw std::runtime_error("'udp_buf_size'=" + std::to_string(udp_buf_size_) + " must NOT be less than '(test_maxbits/8) * 2 + 1024'=" + std::to_string((test_maxbits_/8) * 2 + 1024));
    }

    if (self_cheating_ == true && has_cheater_) {
        std::cerr << "Warning: 'self_cheating' is true, forcing 'has_cheater' to false" << std::endl;
        has_cheater_ = false;
    }

    if (service_ == false && (role_ == "node" || role_ == "cheater")) {
        service_ = true;
        std::cerr << "Warning: 'role' \"node\" or \"cheater\", forcing 'service' to true" << std::endl;
    }
}

void Properties::parse(std::istream & stream) {
    boost::property_tree::ptree ptree;

    boost::property_tree::read_info(stream, ptree);

    port_ = ptree.get("port", port_);
    multicast_group_ = ptree.get("multicast_group", multicast_group_);

    role_ = ptree.get("role", "root");

    timeout_ = ptree.get("timeout", timeout_);
    jump_multi_ = ptree.get("jump_multi", jump_multi_);
    retries_ = ptree.get("retries", retries_);
    enough_servers_available_ = ptree.get("enough_servers_available", enough_servers_available_);
    memory_threshold_ = ptree.get("memory_threshold", memory_threshold_);
    udp_buf_size_ = ptree.get("udp_buf_size", udp_buf_size_);
    thread_poll_size_ = ptree.get("thread_poll_size", thread_poll_size_);

    std::string log_level = ptree.get("log_level", "info");
    if (log_level == "trace") {
        log_level_ = boost::log::trivial::trace;
    } else if (log_level == "debug") {
        log_level_ = boost::log::trivial::debug;
    } else if (log_level == "info") {
        log_level_ = boost::log::trivial::info;
    } else if (log_level == "warning") {
        log_level_ = boost::log::trivial::warning;
    } else if (log_level == "error") {
        log_level_ = boost::log::trivial::error;
    } else if (log_level == "fatal") {
        log_level_ = boost::log::trivial::fatal;
    } else {
        throw std::runtime_error(std::string("Invalid log_level") + log_level);
    }

    root_uuid_ = ptree.get("root_uuid", root_uuid_);

    force_maxsize_ = ptree.get("force_maxsize", force_maxsize_);

    statistics_summary_only_ = ptree.get("statistics.summary_only", statistics_summary_only_);

    multicast_loopback_ = ptree.get("multicast_loopback", multicast_loopback_);

    no_hosting_ = ptree.get("no_hosting", no_hosting_);

    self_cheating_ = ptree.get("self_cheating", self_cheating_);

    test_run_ = ptree.get("test.run", test_run_);

    test_maxbits_ = ptree.get("test.maxbits", test_maxbits_);

    test_mode_ = ptree.get("test.mode", test_mode_);

    has_cheater_ = ptree.get("has_cheater", has_cheater_);

    service_ = ptree.get("service", service_);

    Properties::validate();
}

void Properties::print_help(std::ostream & stream) {
    stream << ";Usage example:" << std::endl;
    stream << ";cat << EOF | ./test 1" << std::endl;
    stream << "  ; this is a comment and has not effect" << std::endl;
    stream << "  ; If you dont pass any of these values, defaults are applied" << std::endl;
    stream << "  port " << port_ << std::endl;
    stream << "  multicast_group \"" << multicast_group_ << "\"" << std::endl;
    stream << "  role \"" << role_ << "\"                 ; possible value are root|client|node|cheater. default is root" << std::endl;
    stream << "  timeout " << timeout_ << "                   ; timeout to assume the other peer doesn't exists" << std::endl;
    stream << "  jump_multi " << jump_multi_ << "               ; don't touch that" << std::endl;
    stream << "  retries " << retries_ << "                   ; number of retries if there is an timeout" << std::endl;
    stream << "  enough_servers_available " << enough_servers_available_ << "  ; max number of server to randonly choose to create a node" << std::endl;
    stream << "  memory_threshold " << static_cast<int32_t>(memory_threshold_) << "          ; host nodes while RAM has at least the percentage available. Use 30 for 30% here" << std::endl;
    stream << "  udp_buf_size " << udp_buf_size_ << "          ; (16384 * 2 + 1024) don't touch this" << std::endl;
    stream << "  thread_poll_size " << thread_poll_size_ << std::endl;

    stream << "  log_level \"" <<
            (log_level_ == boost::log::trivial::trace ? "trace" : log_level_ == boost::log::trivial::debug ? "debug" :
            log_level_ == boost::log::trivial::info ? "info" : log_level_ == boost::log::trivial::warning ? "warning" :
            log_level_ == boost::log::trivial::error ? "error" : log_level_ == boost::log::trivial::fatal ? "fatal" :
            "invalid")
           << "\"            ; possible value are trace|debug|info|warning|error|fatal." << std::endl;

    stream << "  root_uuid " << root_uuid_ << "                 ; unique tree id n the network" << std::endl;
    stream << "  force_maxsize " << (force_maxsize_ ? "true" : "false") << "          ; if true all packages (excepts some types of acknoledgment) are set 'to udp_buf_size'" << std::endl;

    stream << "  statistics {" << std::endl;
    stream << "    summary_only " << (statistics_summary_only_ ? "true" : "false") << "        ; probably you want set this to true" << std::endl;
    stream << "  }" << std::endl;

    stream << "  multicast_loopback " << (multicast_loopback_ ? "true" : "false") << "     ; if true a node can host to itself." << std::endl;
    stream << "  no_hosting " << (no_hosting_ ? "true" : "false") << "            ; if role is root or client and you don't want to host nodes" << std::endl;
    stream << "  self_cheating " << (self_cheating_ ? "true" : "false") << "          ; act as a cheater. help in development when running standalone" << std::endl;

    stream << "  test {" << std::endl;
    stream << "    run " << (test_run_ ? "true" : "false") << "                  ; run test cases" << std::endl;
    stream << "    maxbits " << test_maxbits_ << "            ; max number of bits of running test" << std::endl;
    stream << "    mode \"" << test_mode_ << "\"         ; \"performance\" or \"correctness\"  or \"natural\" or \"registry\"" << std::endl;
    stream << "  }" << std::endl;

    stream << "  has_cheater " << (has_cheater_ ? "true" : "false") << "           ; if should wait for cheater ack when allocatting " << std::endl;
    stream << "  service " << (service_ ? "true" : "false") << "               ; if needs a CTRL+C to finish " << std::endl;
    stream << ";EOF" << std::endl;
}


}


/*
cat << EOF | ./test 1
port 33333
multicast_group "225.0.0.37"
timeout 100
EOF
*/
