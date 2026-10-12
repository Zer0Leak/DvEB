
#include <cstdio>
#include <csignal>
#include <errno.h>
#include <cstring>
#include <climits>
#include <unistd.h>

#include <stdexcept>
#include <iostream>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/eventfd.h>
#include <sys/select.h>

#include <boost/log/trivial.hpp>
#include <boost/thread/thread.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include "bigdata/veb/iveb_global.hh"
#include "bigdata/util.hh"
#include "bigdata/server_multicast.hh"
#include "bigdata/globals.hh"
#include "bigdata/tree_rpc.hh"
#include "bigdata/transaction.hh"
#include "bigdata/channel_multicast.hh"
#include "bigdata/tree_rpc_multicast.hh"
#include "bigdata/iprocessor.hh"
#include "bigdata/properties.hh"
#include "bigdata/statistc.hh"

#include <iostream>

namespace bigdata {

ServerMulticast::ServerMulticast(std::shared_ptr<Registry> registry, std::shared_ptr<Factory> factory, std::shared_ptr<IProcessor> processor)
    : registry_(registry)
    , factory_(factory)
    , buffer_(Properties::get_instance()->udp_buf_size()+1)
    , fd_(-1)
    , fd_event_(-1)
    , cancel_(false)
    , tpool_(Properties::get_instance()->thread_poll_size())
    , channel_(std::make_shared<ChannelMulticast>())
    , processor_(processor)
{
    channel_->open();
}

ServerMulticast::~ServerMulticast() {
    channel_->close();
    stop();
    if (fd_ >= 0) ::close(fd_);
    if (fd_event_ >= 0) ::close(fd_event_);
}

void ServerMulticast::start() {
    if (fd_ >= 0) {
        throw std::runtime_error("Server is already running.");
    }

    cancel_ = false;

    struct sockaddr_in addr;
    struct ip_mreq mreq;
    const int yes = 1;
    // const int no = 0;

    if ((fd_=socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)) < 0) {
        throw std::runtime_error("Can't open socket. Reason: " + std::string(strerror(errno)));
    }

    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        throw std::runtime_error("Can't set SO_REUSEADDR. Reason: " + std::string(strerror(errno)));
    }

    const int socket_buff = 128 * 1024 * 1024;
    if (setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &socket_buff, sizeof(socket_buff)) < 0) {
        throw std::runtime_error("Can't set SO_RCVBUF. Reason: " + std::string(strerror(errno)));
    }

    int socket_buff_check = 0;
    socklen_t socket_buff_check_len = sizeof(socket_buff_check);
    if (getsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &socket_buff_check, &socket_buff_check_len) < 0) {
        throw std::runtime_error("Can't get SO_RCVBUF. Reason: " + std::string(strerror(errno)));
    }
    BOOST_LOG_TRIVIAL(trace) << "RCV_BUFF " << socket_buff_check;
    if (2*socket_buff != socket_buff_check) {
        throw std::runtime_error("rcv 2*socket_buff != socket_buff_check: " + std::to_string(2*socket_buff) + " != " + std::to_string(socket_buff_check));
    }

    memset(&addr, 0x00, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // inet_addr(kMulticatGroup)
    addr.sin_port = htons(Properties::get_instance()->port());

    if (bind(fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't bind to port " + std::to_string(Properties::get_instance()->port()) + ". Reason: " + std::string(strerror(errno)));
    }

    memset(&mreq, 0x00, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(Properties::get_instance()->multicast_group().c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't join multicast group '" + Properties::get_instance()->multicast_group() + "'. Reason: " + std::string(strerror(errno)));
    }

    if((fd_event_ = eventfd(0, EFD_CLOEXEC)) < 0) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't open event fd. Reason: " + std::string(strerror(errno)));
    }

    thread_ = std::make_shared<std::thread>([this](){
        BOOST_LOG_SCOPED_THREAD_TAG("ThreadID", boost::this_thread::get_id());

        int max_fd = std::max(fd_, fd_event_);
        struct sockaddr_in addr;

        while (!cancel_) {
            ssize_t nbytes;

            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(fd_, &rfds);
            FD_SET(fd_event_, &rfds);

            BOOST_LOG_TRIVIAL(trace) << "Server Selecting";
            int rsel = select(max_fd + 1, &rfds, NULL, NULL, NULL);
            BOOST_LOG_TRIVIAL(trace) << "Server Select done. rsel " << rsel;

            if (rsel > 0) {
                if (FD_ISSET(fd_event_, &rfds)) {
                    BOOST_LOG_TRIVIAL(debug) << "Select got eventfd to close Server";
                    uint64_t aux = 0;
                    read(fd_event_, &aux, 8);
                    break;
                }

                while (true) {
                    socklen_t addrlen = sizeof(addr);
                    memset(&addr, 0x00, sizeof(addr));
                    addr.sin_family = AF_INET;
                    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // inet_addr(kMulticatGroup)
                    addr.sin_port = htons(Properties::get_instance()->port());

                    BOOST_LOG_TRIVIAL(trace) << "Server reading";
                    nbytes = recvfrom(fd_, buffer_.data(), buffer_.size(), 0, reinterpret_cast<struct sockaddr*>(&addr), &addrlen);
                    BOOST_LOG_TRIVIAL(trace) << "Server read " << nbytes << " bytes";

                    if (nbytes == 0) {
                        break;
                    }
                    if (nbytes == -1) {
                        if (errno ==  EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            BOOST_LOG_TRIVIAL(fatal) << "Server Can't read from socket. Reason: " << std::string(strerror(errno));
                        }
                    }

                    if (nbytes == static_cast<ssize_t>(buffer_.size())) {
                        BOOST_LOG_TRIVIAL(error) << "Not expected to read more than " << Properties::get_instance()->udp_buf_size() << " bytes";
                    }

                    if (Properties::get_instance()->multicast_loopback() == false) {
                        if (std::dynamic_pointer_cast<ChannelMulticast>(channel_)->src_addr().addr_.sin_addr.s_addr == addr.sin_addr.s_addr /* &&
                                    std::dynamic_pointer_cast<ChannelMulticast>(channel_)->src_addr().addr_.sin_port == addr.sin_port */) {
                            BOOST_LOG_TRIVIAL(trace) << "ignoring to myself " << inet_ntoa(addr.sin_addr);
                            continue;
                        }
                    }
                    try {
                        std::vector<uint8_t> buffer(nbytes);
                        std::copy(buffer_.begin(), buffer_.begin()+nbytes, buffer.begin());

                        ServerDispatchArgs sda;
                        TreeRpc::Method method = TreeRpc::kInvalid;
                        TreeRpcMulticast::parse(method, sda, buffer, true);

                        BOOST_LOG_TRIVIAL(trace) << "Server enqueing " << method << " for " << sda.transaction;
                        tpool_.enqueue(&ServerMulticast::process, this, std::move(buffer), std::make_shared<AddressInfoMulticast>(&addr));
                        BOOST_LOG_TRIVIAL(trace) << "Server "  << method << " for " << sda.transaction << " enqueing done";
                    } catch (const std::runtime_error & e) {
                        BOOST_LOG_TRIVIAL(error) << "Exception trying to enqueue into pool" << e.what();
                    }

                }
            } else {
                BOOST_LOG_TRIVIAL(error) << "Server can't select from socket. Reason: " << std::string(strerror(errno));
            }
        }
        BOOST_LOG_TRIVIAL(debug) << "Server is leaving its thread";
    });
}

void ServerMulticast::stop() {
    cancel_ = true;
    ServerDispatchArgs sda;
    notify_all_force(TreeRpc::kControlForceStop, sda);
    if (fd_event_ >= 0) {
        uint64_t buf = 1;
        if (8 != write(fd_event_, &buf, 8)) {
            throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't event to stop. Reason: " + std::string(strerror(errno)));
        }
    }
}

void ServerMulticast::join() {
    if (thread_ != NULL) {
        thread_->join();
    }
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    if (fd_event_ >= 0) {
        ::close(fd_event_);
        fd_event_ = -1;
    }
}

void ServerMulticast::process(std::vector<uint8_t> buffer, std::shared_ptr<AddressInfo> addr) {
    auto sample = statistic::include_thread_sample();
    try {
        processor_->process(std::move(buffer), addr, std::dynamic_pointer_cast<Server>(this->shared_from_this()));
    } catch (const std::runtime_error & e) {
        BOOST_LOG_TRIVIAL(error) << "Process exception: " << e.what();
    }
}

}
