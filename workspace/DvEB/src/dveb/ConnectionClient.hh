/*
 * SocketClient.hh
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#ifndef DVEB_CONNECTIONCLIENT_HH_
#define DVEB_CONNECTIONCLIENT_HH_

#include <string>
#include <cstdint>
#include <memory>

#include "dveb/Socket.hh"

class ConnectionClient {
public:
  virtual ~ConnectionClient() {}

  virtual void connect(const std::string & address, uint16_t port) = 0;

  virtual std::shared_ptr<Socket> socket() = 0;
};

#endif /* DVEB_CONNECTIONCLIENT_HH_ */
