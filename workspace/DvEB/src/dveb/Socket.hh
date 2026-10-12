/*
 * Socket.hh
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#ifndef DVEB_SOCKET_HH_
#define DVEB_SOCKET_HH_

#include <cstddef>
#include <utility>
#include <cstdio>
#include <cstdint>

class Socket {
public:
  virtual ~Socket() {}

  virtual ssize_t read_all(void * data, size_t max_size) = 0;
  virtual ssize_t write(const void *buffer, size_t bsize) = 0;
  virtual void close() = 0;
  virtual std::string info() = 0;
};

#endif /* DVEB_SOCKET_HH_ */
