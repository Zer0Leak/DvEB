/*
 * SocketServer.hh
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#ifndef DVEB_CONNECTIONSERVER_HH_
#define DVEB_CONNECTIONSERVER_HH_

#include <memory>

#include "dveb/Socket.hh"

class ConnectionServer {
public:
  virtual ~ConnectionServer() {}

  // std::shared_ptr<Socket> accept() = 0;
};

#endif /* DVEB_CONNECTIONSERVER_HH_ */
