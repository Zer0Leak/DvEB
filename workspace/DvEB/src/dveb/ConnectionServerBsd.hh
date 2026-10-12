/*
 * ConnectionServerBsd.hh
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#ifndef DVEB_CONNECTIONSERVERBSD_HH_
#define DVEB_CONNECTIONSERVERBSD_HH_

#include "ConnectionServer.hh"

class ProtoVEB;

class ConnectionServerBsd : public ConnectionServer {
public:
  ConnectionServerBsd(uint16_t port, std::shared_ptr<ProtoVEB> pvEB_);
  virtual ~ConnectionServerBsd();

  void start();

private:
  uint16_t port_;
  std::shared_ptr<ProtoVEB> pvEB_;
};

#endif /* DVEB_CONNECTIONSERVERBSD_HH_ */
