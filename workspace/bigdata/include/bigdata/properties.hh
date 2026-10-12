
#ifndef BIGDATA_PROPERTIES_HH_
#define BIGDATA_PROPERTIES_HH_

#include <cstdint>

#include <string>
#include <memory>
#include <mutex>
#include <iostream>

#include <boost/log/trivial.hpp>

namespace bigdata {

class Properties {
public:
    static std::shared_ptr<Properties> get_instance();

    uint16_t port() const { return port_; }
    void set_port(uint16_t port) { port_ = port; }

    int32_t timeout() const { return timeout_; }
    void set_timeout(uint16_t timeout) { timeout_ = timeout; }

    int32_t jump_multi() const { return jump_multi_; }
    void set_jump_multi(int32_t jump_multi) { jump_multi_ = jump_multi; }

    int32_t retries() const { return retries_; }
    void set_retries(int32_t retries) { retries_ = retries; }

    const std::string & multicast_group() const { return multicast_group_; }
    void set_multicast_group(const std::string & multicast_group) { multicast_group_ = multicast_group; }
    void set_multicast_group(std::string && multicast_group) { multicast_group_ = multicast_group; }

    const std::string & role() const { return role_; }
    void role(const std::string & role) { role_ = role; }
    void role(std::string && role) { role = role; }

    uint32_t enough_servers_available() const { return enough_servers_available_; }
    void set_enough_servers_available(uint32_t enough_servers_available) { enough_servers_available_ = enough_servers_available; }

    uint8_t memory_threshold() const { return memory_threshold_; }
    void set_memory_threshold(uint8_t memory_threshold) { memory_threshold_ = memory_threshold; }

    uint32_t udp_buf_size() const { return udp_buf_size_ ; }
    void set_udp_buf_size(uint32_t udp_buf_size) { udp_buf_size_ = udp_buf_size; }

    uint16_t thread_poll_size() const { return thread_poll_size_ ; }
    void set_thread_poll_size(uint16_t thread_poll_size) { thread_poll_size_ = thread_poll_size; }

    boost::log::trivial::severity_level log_level() const { return log_level_; }
    void set_log_level(boost::log::trivial::severity_level log_level) { log_level_ = log_level; }

    uint32_t root_uuid() const { return root_uuid_ ; }
    void set_root_uuid(uint32_t root_uuid) { root_uuid_ = root_uuid; }

    bool force_maxsize() const { return force_maxsize_ ; }
    void set_force_maxsize(bool force_maxsize) { force_maxsize_ = force_maxsize; }

    bool statistics_summary_only() const { return statistics_summary_only_; }
    void set_statistics_summary_only(bool statistics_summary_only) { statistics_summary_only_ = statistics_summary_only; }

    bool multicast_loopback() const { return multicast_loopback_; }
    void set_multicast_loopback(bool multicast_loopback) { multicast_loopback_ = multicast_loopback; }

    bool no_hosting() const { return no_hosting_; }
    void set_no_hosting(bool no_hosting) { no_hosting_ = no_hosting; }

    bool self_cheating() const { return self_cheating_; }
    void set_self_cheating(bool self_cheating) { self_cheating_ = self_cheating; }

    bool test_run() const { return test_run_; }
    void set_test_run(bool test_run) { test_run_ = test_run; }

    uint32_t test_maxbits() const { return test_maxbits_ ; }
    void set_test_maxbits(uint32_t test_maxbits) { test_maxbits_ = test_maxbits; }

    const std::string & test_mode() const { return test_mode_ ; }
    void set_test_mode(const std::string & test_mode) { test_mode_ = test_mode; }


    bool has_cheater() const { return has_cheater_; }
    void set_has_cheater(bool has_cheater) { has_cheater_ = has_cheater; }

    bool service() const { return service_; }
    void set_service(bool service) { service_ = service; }

    void parse(std::istream & stream);

    void validate();

    void print_help(std::ostream & stream);


private:
    static std::shared_ptr<Properties> singletoon_;
    Properties();
    static std::mutex mutex_;

    uint16_t port_;
    std::string multicast_group_;
    std::string role_;
    int32_t timeout_;
    int32_t jump_multi_;
    int32_t retries_;
    uint32_t enough_servers_available_;
    uint8_t memory_threshold_;
    uint32_t udp_buf_size_;
    uint16_t thread_poll_size_;
    boost::log::trivial::severity_level log_level_;
    uint32_t root_uuid_;
    bool force_maxsize_;
    bool statistics_summary_only_;
    bool multicast_loopback_;
    bool no_hosting_;
    bool self_cheating_;
    bool test_run_;
    uint32_t test_maxbits_;
    std::string test_mode_;
    bool has_cheater_;
    bool service_;

};

}

#endif  // BIGDATA_PROPERTIES_HH_
