/*
 * SocketBsd.cc
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>

#include <cstring>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/file.h>

#include <stdexcept>
#include <iostream>

#include "dveb/SocketBsd.hh"
#include "dveb/globals.hh"

SocketBsd::SocketBsd() : socket_(-1), cached_(DVEB_CACHE_SOCKET), port_(0) {

}

SocketBsd::SocketBsd(int sock, bool cached): socket_(sock), cached_(cached), port_(0) {

}

SocketBsd::SocketBsd(const std::string & address, uint16_t port) : socket_(-1), cached_(DVEB_CACHE_SOCKET) {
  struct sockaddr_in sadr_server;
  int sock_opt_flag = 1;
  int sock = -1;

  memset(&sadr_server, 0x00, sizeof(struct sockaddr_in));
  sadr_server.sin_family = AF_INET;
  sadr_server.sin_addr.s_addr = inet_addr(address.c_str());
  sadr_server.sin_port = htons(port);

  if ((sock = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    throw std::runtime_error("Can't create socket" + std::string(strerror(errno)));
  }

  // disabling nagle
  if (0 != setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &sock_opt_flag, sizeof(sock_opt_flag))) {
    ::close(sock);
    throw std::runtime_error("Can't disable nagle" + std::string(strerror(errno)));
  }

  if (::connect(sock, (struct sockaddr *) &sadr_server, sizeof(sadr_server)) < 0) {
    ::close(sock);
    throw std::runtime_error("Can't connect to " + address + ":" + std::to_string(port) + ". " + std::string(strerror(errno)));
  }

  address_ = address;
  port_ = port;

  socket_ = sock;
}

SocketBsd::~SocketBsd() {
  // std::cout << "deleting socket" << std::endl;
  close(false);
}

ssize_t SocketBsd::read_all(void * data, size_t max_size) {
  ssize_t count = 0;

  if (socket_ == -1) {
    return -1;
  }

  while(count < (ssize_t)max_size) {
    ssize_t aux = recv(socket_, (uint8_t*)data + count, max_size - count, 0);
    if (aux == -1) {
      std::cerr << "Can't read. error=[" << std::to_string(errno) << ", '" << std::string(strerror(errno)) << "']." << std::endl;
      return -1;
    }
    if (aux == 0) {
      std::cerr << "recv returned 0=[" << std::to_string(errno) << ", '" << std::string(strerror(errno)) << "']." << std::endl;
      return -1;
    }
    count += aux;
  }

  return count;
}

ssize_t SocketBsd::write(const void *buffer, size_t bsize) {
  ssize_t count = 0;

  if (socket_ == -1) {
    return -1;
  }

  while (count < (ssize_t)bsize) {

    ssize_t aux = send(socket_, (uint8_t*)buffer + count, bsize - count, 0);

    if (aux == -1) {
      return -1;
    }
    count += aux;
  }

  return count;
}

void SocketBsd::close() {
  close(cached_);
}

void SocketBsd::close(bool cached) {
  if (false == cached) {
    if (socket_ != -1) {
      ::shutdown(socket_, SHUT_RDWR);
      char ch;
      ::recv(socket_, &ch, 1, 0);
      ::close(socket_);
      socket_ = -1;
      // std::cout << "shutdown" << std::endl;
    }
  }
}

std::string SocketBsd::info() {
  return address_ + ":" + std::to_string(port_);
}

