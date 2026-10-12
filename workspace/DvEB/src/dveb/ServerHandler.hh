/*
 * ServerHandler.hh
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#ifndef SERVERHANDLER_HH_
#define SERVERHANDLER_HH_

#include <memory>
#include <cstdint>

#include "dveb/DvebRpc.hh"

class Socket;
class ProtoVEB;

class ServerHandler {
public:
  ServerHandler();
  virtual ~ServerHandler();

  bool handle(Socket * sock, std::shared_ptr<ProtoVEB> pVEB);

private:

  std::shared_ptr<DvebRpc::RpcCallPtr> readMethod(Socket * sock);

  void writeAnswer(Socket * sock, DvebRpc::RpcAnswer * answer);

  std::shared_ptr<DvebRpc::RpcAnswerPtr> RunMethodK(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB);

  std::shared_ptr<DvebRpc::RpcAnswerPtr> RunMethodGet(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB);

  std::shared_ptr<DvebRpc::RpcAnswerPtr> RunMethodGetMin(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB);

  std::shared_ptr<DvebRpc::RpcAnswerPtr> RunMethodGetMax(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB);

  std::shared_ptr<DvebRpc::RpcAnswerPtr> RunMethodGetSuccessor(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB);

  std::shared_ptr<DvebRpc::RpcAnswerPtr> RunMethodGetPredecessor(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB);

  std::shared_ptr<DvebRpc::RpcAnswerPtr> RunMethodRemove(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB);

  std::shared_ptr<DvebRpc::RpcAnswerPtr> RunMethodInsert(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB);

  std::shared_ptr<DvebRpc::RpcAnswerPtr> RunMethodStatus(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB);
};

#endif /* SERVERHANDLER_HH_ */
