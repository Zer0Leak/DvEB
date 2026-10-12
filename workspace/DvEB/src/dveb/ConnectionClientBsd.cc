/*
 * ConnectionClientBsd.cc
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <utility>


#include <map>

#include <stdexcept>

#include "ConnectionClientBsd.hh"
#include "dveb/globals.hh"

#include <string>

ConnectionClientBsd::ConnectionClientBsd() : port_(0) {
}

ConnectionClientBsd::~ConnectionClientBsd() {
  disconnect();
}

static std::map<std::string, std::shared_ptr<Socket>> s_open_socket_;

void ConnectionClientBsd::connect(const std::string & address, uint16_t port) {
  address_ = address;
  port_ = port;

  if (DVEB_CACHE_SOCKET) {
    std::string hashname = address + std::string(";;") + std::to_string(port);

    std::map<std::string, std::shared_ptr<Socket>>::iterator it = s_open_socket_.find(hashname);
    if (it != s_open_socket_.end()) {
      bsd_socket_ = it->second;
      return;
    }
    bsd_socket_.reset(new SocketBsd(address_, port_));
    s_open_socket_.insert(std::make_pair(hashname, bsd_socket_));
  } else {
    bsd_socket_.reset(new SocketBsd(address_, port_));
  }
}

void ConnectionClientBsd::disconnect() {
  bsd_socket_.reset();
}

std::shared_ptr<Socket> ConnectionClientBsd::socket() {
  return bsd_socket_;
}

