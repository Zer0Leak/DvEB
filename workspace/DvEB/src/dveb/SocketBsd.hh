/*
 * SocketBsd.hh
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#ifndef DVEB_SOCKETBSD_HH_
#define DVEB_SOCKETBSD_HH_

#include <string>

#include "dveb/Socket.hh"


class SocketBsd : public Socket {
public:
  SocketBsd();
  SocketBsd(int sock, bool cached);
  SocketBsd(const std::string & address, uint16_t port);
  virtual ~SocketBsd();

  virtual ssize_t read_all(void * data, size_t max_size);
  virtual ssize_t write(const void *buffer, size_t bsize);
  virtual void close();
  virtual std::string info();

private:
  int socket_;
  bool cached_;
  std::string address_;
  uint16_t port_;

  virtual void close(bool cached);
};

#endif /* DVEB_SOCKETBSD_HH_ */
