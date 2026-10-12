/*
 * ConnectionClientBsd.hh
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#ifndef DVEB_CONNECTIONCLIENTBSD_HH_
#define DVEB_CONNECTIONCLIENTBSD_HH_

#include "dveb/ConnectionClient.hh"
#include "dveb/SocketBsd.hh"

#include <string>
#include <memory>

class ConnectionClientBsd : public ConnectionClient {
public:
  ConnectionClientBsd();
  virtual ~ConnectionClientBsd();

  virtual void connect(const std::string & address, uint16_t port);

  virtual void disconnect();

  virtual std::shared_ptr<Socket> socket();
private:
  std::string address_;
  uint16_t port_;

  std::shared_ptr<Socket> bsd_socket_;
};

#endif /* DVEB_CONNECTIONCLIENTBSD_HH_ */
