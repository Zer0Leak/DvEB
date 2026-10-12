/*
 * ProtoVEBClient.h
 *
 *  Created on: Aug 2, 2015
 *      Author: edgard
 */

#ifndef PROTOVEBCLIENT_HH_
#define PROTOVEBCLIENT_HH_

#include "ProtoVEBk.hh"

#include "dveb/ConnectionClientBsd.hh"
#include "dveb/DvebRpc.hh"

class ProtoVEBClient : public ProtoVEB {
public:
  ProtoVEBClient(const std::string & address, const std::string & port);
  virtual ~ProtoVEBClient();

  virtual uint8_t K();

  virtual std::shared_ptr<ProtoData> get(uint64_t x);
  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMin();
  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMax();
  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getSuccessor(uint64_t x);
  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getPredecessor(uint64_t x);
  virtual std::shared_ptr<ProtoData> insert(uint64_t x, std::shared_ptr<ProtoData> data);
  virtual std::shared_ptr<ProtoData> remove(uint64_t x);

  virtual std::string status();

private:
  std::unique_ptr<DvebRpc::RpcAnswerPtr> transfer(ConnectionClient * conn, std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call);

  std::string address_;
  std::string port_;
};

#endif /* PROTOVEBCLIENT_HH_ */
