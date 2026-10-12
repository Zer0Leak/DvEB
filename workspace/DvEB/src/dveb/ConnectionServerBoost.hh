/*
 * SocketServerBoost.hh
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#ifndef DVEB_CONNECTIONSERVERBOOST_HH_
#define DVEB_CONNECTIONSERVERBOOST_HH_

#include "ConnectionServer.hh"

class ConnectionServerBoost : public ConnectionServer {
public:
  ConnectionServerBoost();
  virtual ~ConnectionServerBoost();

  std::shared_ptr<Socket> accept();
};

#endif /* DVEB_CONNECTIONSERVERBOOST_HH_ */
