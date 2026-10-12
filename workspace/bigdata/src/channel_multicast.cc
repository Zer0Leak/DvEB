#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <error.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/eventfd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <net/if.h>


#include <signal.h>

#include <cstdint>
#include <climits>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/log/trivial.hpp>

#include "bigdata/channel_multicast.hh"
#include "bigdata/globals.hh"
#include "bigdata/properties.hh"

namespace bigdata {

AddressInfoMulticast::AddressInfoMulticast(const struct sockaddr_in * addr) {
    if (NULL != addr) {
        memcpy(&addr_, addr, sizeof(addr_));
    } else {
        memset(&addr_, 0x00, sizeof(addr_));
    }
}

AddressInfoMulticast::AddressInfoMulticast(const AddressInfoMulticast & addr) {
    memcpy(&addr_, &addr.addr_, sizeof(addr_));
}

AddressInfoMulticast::AddressInfoMulticast(AddressInfoMulticast && addr) {
    memcpy(&addr_, &addr.addr_, sizeof(addr_));
    memset(&addr.addr_, 0x00, sizeof(addr_));
}

AddressInfoMulticast & AddressInfoMulticast::operator=(const AddressInfoMulticast & addr) {
    memcpy(&addr_, &addr.addr_, sizeof(addr_));
    return *this;
}

ChannelMulticast::ChannelMulticast()
    : buffer_(Properties::get_instance()->udp_buf_size())
    , fd_(-1)
    , src_addr_(NULL)
    , file_(NULL)
{
    memset(&addr_, 0x00, sizeof(addr_));
}

ChannelMulticast::~ChannelMulticast() {
    close();
}

void ChannelMulticast::process(const AddressInfoMulticast * unicast_addr, bool force_min_size) {

    if (fd_ != -1) {
        ssize_t rbytes = ::ftell(file_);
        ssize_t wbytes = 0;

        if (rbytes > 0) {

            if (force_min_size == false && Properties::get_instance()->force_maxsize()) {
                if (rbytes < static_cast<ssize_t>(buffer_.size())) {
                    memset(buffer_.data()+rbytes, 0x00, buffer_.size()-rbytes);
                }

                rbytes = static_cast<ssize_t>(buffer_.size());
            }

            ::rewind(file_);

            if (unicast_addr != NULL) {
                BOOST_LOG_TRIVIAL(trace) << "Sending to " << inet_ntoa(unicast_addr->addr_.sin_addr) << ":" << unicast_addr->addr_.sin_port;
                wbytes = sendto(fd_, buffer_.data(), rbytes, 0, reinterpret_cast<const struct sockaddr*>(&unicast_addr->addr_), sizeof(unicast_addr->addr_));
            } else {
                BOOST_LOG_TRIVIAL(trace) << "Sending to " << inet_ntoa(addr_.sin_addr) << ":" << addr_.sin_port;
                wbytes = sendto(fd_, buffer_.data(), rbytes, 0, reinterpret_cast<struct sockaddr*>(&addr_), sizeof(addr_));
            }

            if (wbytes != rbytes) {
                BOOST_LOG_TRIVIAL(error) << "Can't sendto socket. Reason: " << strerror(errno);
                /*
                    try {
                        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't sendto socket. Reason: " + std::string(strerror(errno)));
                    } catch (const std::runtime_error & e) {
                        std::cerr << e.what() << std::endl;
                    }
                    */
            } else {
                BOOST_LOG_TRIVIAL(trace) << rbytes << " bytes sent";
            }

        } else if (rbytes < 0) {
            BOOST_LOG_TRIVIAL(fatal) << "Can't ftell buffer. Reason: " << strerror(errno);
            ::kill(getpid(), SIGINT);
            return;
            /*
                try {
                    throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't ftell buffer. Reason: " + std::string(strerror(errno)));
                } catch (const std::runtime_error & e) {
                    std::cerr << e.what() << std::endl;
                    abort();
                }
                */
        }

    }
}

void ChannelMulticast::get_mac(int sock) {
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't get MAC address (SIOCGIFCONF). Reason: " + std::string(strerror(errno)));
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    // TODO
                    memcpy(mac_address_, ifr.ifr_hwaddr.sa_data, 6);
                    if (ioctl(sock, SIOCGIFADDR, &ifr) != 0) {
                        BOOST_LOG_TRIVIAL(fatal) << "SIOCGIFADDR failed. Reason: " << strerror(errno);
                        throw std::runtime_error("SIOCGIFADDR failed. Reason: " + std::string(strerror(errno)));
                    }
                    memcpy(&src_addr_.addr_, &ifr.ifr_addr, sizeof(src_addr_.addr_));
                    return;
                }
            }
        } else {
            throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't get MAC address (SIOCGIFFLAGS). Reason: " + std::string(strerror(errno)));
        }
    }

    throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't get MAC address.");
}

void ChannelMulticast::open() {
    try {
        const int yes = 1;
        const int no = 0;

        std::lock_guard<std::recursive_mutex>lk(mutex_app_);

        if (file_ != NULL) return; // already openned

        file_ = fmemopen(buffer_.data(), buffer_.size(), "rb+");
        if (file_ == NULL) {
            BOOST_LOG_TRIVIAL(fatal) << "ChannelMulticast can't fdopen pipe[1]. Reason: " << strerror(errno);
            throw std::runtime_error("ChannelMulticast can't fdopen pipe[1]. Reason: " + std::string(strerror(errno)));
        }

        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = inet_addr(Properties::get_instance()->multicast_group().c_str());
        addr_.sin_port = htons(Properties::get_instance()->port());

        if ((fd_ = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
            BOOST_LOG_TRIVIAL(fatal) << "Can't connect pipe[1]. Reason: " << strerror(errno);
            throw std::runtime_error("Can't connect. Reason: " + std::string(strerror(errno)));
        }

        int32_t ttl = 1;
        if (setsockopt(fd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) != 0) {
            BOOST_LOG_TRIVIAL(fatal) << "Can't set TTL. Reason: " << strerror(errno);
            throw std::runtime_error("Can't set TTL. Reason: " + std::string(strerror(errno)));
        }

        if (Properties::get_instance()->multicast_loopback()) {
            if(setsockopt(fd_, IPPROTO_IP, IP_MULTICAST_LOOP, &yes, sizeof(yes)) < 0) {
                throw std::runtime_error("Can't enable loopback. Reason: " + std::string(strerror(errno)));
            }
        } else {
            if(setsockopt(fd_, IPPROTO_IP, IP_MULTICAST_LOOP, &no, sizeof(no)) < 0) {
                throw std::runtime_error("Can't disable loopback. Reason: " + std::string(strerror(errno)));
            }
        }

        const int socket_buff = 128 * 1024 * 1024;
        if (setsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &socket_buff, sizeof(socket_buff)) < 0) {
            throw std::runtime_error("Can't set SO_SNDBUF. Reason: " + std::string(strerror(errno)));
        }

        int socket_buff_check = 0;
        socklen_t socket_buff_check_len = sizeof(socket_buff_check);
        if (getsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &socket_buff_check, &socket_buff_check_len) < 0) {
            throw std::runtime_error("Can't get SO_SNDBUF. Reason: " + std::string(strerror(errno)));
        }
        BOOST_LOG_TRIVIAL(trace) << "SND_BUFF " << socket_buff_check;
        if (2*socket_buff != socket_buff_check) {
            throw std::runtime_error("snd 2*socket_buff != socket_buff_check: " + std::to_string(2*socket_buff) + " != " + std::to_string(socket_buff_check));
        }


        //socklen_t len = sizeof(src_addr_.addr_);
        //if (getsockname(fd_, (struct sockaddr *)(&src_addr_.addr_), &len) != 0) {
        //    BOOST_LOG_TRIVIAL(fatal) << "getsockname failed. Reason: " << strerror(errno);
        //    throw std::runtime_error("getsockname failed. Reason: " + std::string(strerror(errno)));
        //}

        get_mac(fd_);
    } catch (const std::runtime_error & e) {
        close();
        throw e;
    }
}

void ChannelMulticast::close() {
    std::lock_guard<std::recursive_mutex>lk(mutex_app_);

    if (file_ == NULL) return; // already closed

    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }

    if (file_ != NULL) {
        ::fclose(file_);
        file_ = NULL;
    }
}


ChannelMulticast::operator FILE*() {
    std::lock_guard<std::recursive_mutex>lk(mutex_app_);
    return file_;
}

std::recursive_mutex & ChannelMulticast::mutex() {
    return mutex_app_;
}

void ChannelMulticast::sync(std::shared_ptr<const AddressInfo> unicast_addr, bool force_min_size) {
    std::lock_guard<std::recursive_mutex>lk(mutex_app_);
    fflush(file_);
    auto aux = std::dynamic_pointer_cast<const AddressInfoMulticast>(unicast_addr);
    process(aux.get(), force_min_size);
}

}
