/*
 * ServerHandler.cc
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */
#include <cstdlib>
#include <ctime>

#include <sys/sysinfo.h>

#include "ServerHandler.hh"
#include "ProtoVEB.hh"
#include "ProtoVEBClient.hh"

#include "Socket.hh"

#include "ProtoDataRaw.hh"

#ifdef _WIN32
using boost::asio::ip::tcp;
#include <boost/detail/endian.hpp>
#include <cstdlib>
#else
#include <endian.h>
#endif

ServerHandler::ServerHandler() {
  // TODO Auto-generated constructor stub

}

ServerHandler::~ServerHandler() {
  // TODO Auto-generated destructor stub
}

std::shared_ptr<DvebRpc::RpcCallPtr> ServerHandler::readMethod(Socket * sock) {
  uint64_t method_size;
  ssize_t read_size;

  //std::cerr << "b4 read" << std::endl;
  read_size =  sock->read_all(&method_size, sizeof(uint64_t));
  //std::cerr << "after read" << std::endl;

  if (read_size != sizeof(uint64_t)) {
    throw std::runtime_error("method header couldn't be read. read_size = " + std::to_string(read_size));
  }

  method_size = be64toh(method_size);

  if (method_size < sizeof(DvebRpc::RpcCall)) {
    throw std::runtime_error(std::string("invalid method_size = ") + std::to_string(method_size));
  }

  std::shared_ptr<DvebRpc::RpcCallPtr> method(new DvebRpc::RpcCallPtr(method_size - sizeof(DvebRpc::RpcCall)));

  read_size =  sock->read_all((uint8_t*)method->rpc_ + sizeof(uint64_t), method_size - sizeof(method_size));

  if (read_size != static_cast<ssize_t>(method_size - sizeof(method_size))) {
    throw std::runtime_error("method couldn't be read. read_size = " + std::to_string(read_size));
  }

  return method;
}

void ServerHandler::writeAnswer(Socket * sock, DvebRpc::RpcAnswer * answer) {
  uint64_t write_size = answer->size;
  answer->size = be64toh(answer->size);
  sock->write(answer, write_size);
}

std::shared_ptr<DvebRpc::RpcAnswerPtr> ServerHandler::RunMethodK(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB) {
  uint8_t k = pVEB->K();

  std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(0));
  answer->answ_->k = k;

  return answer;
}

std::shared_ptr<DvebRpc::RpcAnswerPtr> ServerHandler::RunMethodGet(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB) {
  std::shared_ptr<ProtoDataRaw> pd = std::dynamic_pointer_cast<ProtoDataRaw>(pVEB->get(pbMethod->x));

  if (pd != NULL) {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(pd->size()));
    answer->answ_->is_valid = true;
    answer->answ_->x = pbMethod->x;
    answer->answ_->size = sizeof(DvebRpc::RpcAnswer) + pd->size();
    memcpy(DvebRpc::GetData(answer->answ_), pd->data(), pd->size());
    return answer;
  } else {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(0));
    answer->answ_->is_valid = false;
    return answer;
  }

}


std::shared_ptr<DvebRpc::RpcAnswerPtr> ServerHandler::RunMethodGetMin(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB) {
  uint64_t x;
  std::shared_ptr<ProtoData> pd;
  std::tie(x, pd) = pVEB->getMin();

  if (pd != NULL) {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(pd->size()));
    answer->answ_->is_valid = true;
    answer->answ_->x = x;
    answer->answ_->size = sizeof(DvebRpc::RpcAnswer) + pd->size();
    memcpy(DvebRpc::GetData(answer->answ_), pd->data(), pd->size());
    return answer;
  } else {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(0));
    answer->answ_->is_valid = false;
    return answer;
  }
}

std::shared_ptr<DvebRpc::RpcAnswerPtr> ServerHandler::RunMethodGetMax(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB) {
  uint64_t x;
  std::shared_ptr<ProtoData> pd;
  std::tie(x, pd) = pVEB->getMax();

  if (pd != NULL) {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(pd->size()));
    answer->answ_->is_valid = true;
    answer->answ_->x = x;
    answer->answ_->size = sizeof(DvebRpc::RpcAnswer) + pd->size();
    memcpy(DvebRpc::GetData(answer->answ_), pd->data(), pd->size());
    return answer;
  } else {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(0));
    answer->answ_->is_valid = false;
    return answer;
  }
}

std::shared_ptr<DvebRpc::RpcAnswerPtr> ServerHandler::RunMethodGetSuccessor(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB) {
  uint64_t x;
  std::shared_ptr<ProtoData> pd;
  std::tie(x, pd) = pVEB->getSuccessor(pbMethod->x);

  if (pd != NULL) {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(pd->size()));
    answer->answ_->is_valid = true;
    answer->answ_->x = x;
    answer->answ_->size = sizeof(DvebRpc::RpcAnswer) + pd->size();
    memcpy(DvebRpc::GetData(answer->answ_), pd->data(), pd->size());
    return answer;
  } else {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(0));
    answer->answ_->is_valid = false;
    return answer;
  }
}

std::shared_ptr<DvebRpc::RpcAnswerPtr> ServerHandler::RunMethodGetPredecessor(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB) {
  uint64_t x;
  std::shared_ptr<ProtoData> pd;
  std::tie(x, pd) = pVEB->getPredecessor(pbMethod->x);

  if (pd != NULL) {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(pd->size()));
    answer->answ_->is_valid = true;
    answer->answ_->x = x;
    answer->answ_->size = sizeof(DvebRpc::RpcAnswer) + pd->size();
    memcpy(DvebRpc::GetData(answer->answ_), pd->data(), pd->size());
    return answer;
  } else {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(0));
    answer->answ_->is_valid = false;
    return answer;
  }
}

std::shared_ptr<DvebRpc::RpcAnswerPtr> ServerHandler::RunMethodRemove(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB) {
  std::shared_ptr<ProtoData> pd;
  pd = pVEB->remove(pbMethod->x);

  if (pd != NULL) {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(pd->size()));
    answer->answ_->is_valid = true;
    answer->answ_->x = pbMethod->x;
    answer->answ_->size = sizeof(DvebRpc::RpcAnswer) + pd->size();
    memcpy(DvebRpc::GetData(answer->answ_), pd->data(), pd->size());
    return answer;
  } else {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(0));
    answer->answ_->is_valid = false;
    return answer;
  }
}

std::shared_ptr<DvebRpc::RpcAnswerPtr> ServerHandler::RunMethodInsert(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB) {
  std::shared_ptr<ProtoDataRaw> pdr(new ProtoDataRaw());
  std::shared_ptr<ProtoData> pd;

  pdr->Set(DvebRpc::GetData((DvebRpc::RpcCall *)pbMethod), DvebRpc::GetDataSize(pbMethod));

  pd = pVEB->insert(pbMethod->x, pdr);

  if (pd != NULL) {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(pd->size()));
    answer->answ_->is_valid = true;
    answer->answ_->x = pbMethod->x;
    answer->answ_->size = sizeof(DvebRpc::RpcAnswer) + pd->size();
    memcpy(DvebRpc::GetData(answer->answ_), pd->data(), pd->size());
    return answer;
  } else {
    std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(0));
    answer->answ_->is_valid = false;
    return answer;
  }
}

std::shared_ptr<DvebRpc::RpcAnswerPtr> ServerHandler::RunMethodStatus(const DvebRpc::RpcCall * pbMethod, std::shared_ptr<ProtoVEB> pVEB) {
  std::string pd;

  pd += "Process memory in bytes: " + FormatWithCommas(getUsedMemory());
  struct sysinfo info;
  sysinfo(&info);
  pd += ", Free system memory in bytes: " + FormatWithCommas(info.freeram) + "/" + FormatWithCommas(info.totalram) + " {";

  pd += pVEB->status();
  pd += "}";

  std::shared_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(pd.size() + 1));
  answer->answ_->is_valid = true;
  answer->answ_->size = sizeof(DvebRpc::RpcAnswer) + pd.size() + 1;
  memcpy(DvebRpc::GetData(answer->answ_), pd.c_str(), pd.size()+1);
  return answer;
}

bool ServerHandler::handle(Socket * sock, std::shared_ptr<ProtoVEB> pVEB)
{
  std::shared_ptr<DvebRpc::RpcAnswerPtr> answerGet;
  bool ret = false;

  std::shared_ptr<DvebRpc::RpcCallPtr> pbMethod;

  try
  {
    pbMethod = readMethod(sock);

    if (pbMethod->rpc_->method == DvebRpc::kMGet) {
      answerGet = RunMethodGet(pbMethod->rpc_, pVEB);
    } else if (pbMethod->rpc_->method == DvebRpc::kMGetMin) {
      answerGet = RunMethodGetMin(pbMethod->rpc_, pVEB);
    } else if (pbMethod->rpc_->method == DvebRpc::kMGetMax) {
      answerGet = RunMethodGetMax(pbMethod->rpc_, pVEB);
    } else if (pbMethod->rpc_->method == DvebRpc::kMGetSuccessor) {
      answerGet = RunMethodGetSuccessor(pbMethod->rpc_, pVEB);
    } else if (pbMethod->rpc_->method == DvebRpc::kMGetPredecessor) {
      answerGet = RunMethodGetPredecessor(pbMethod->rpc_, pVEB);
    } else if (pbMethod->rpc_->method == DvebRpc::kMRemove) {
      answerGet = RunMethodRemove(pbMethod->rpc_, pVEB);
    } else if (pbMethod->rpc_->method == DvebRpc::kMInsert) {
      answerGet = RunMethodInsert(pbMethod->rpc_, pVEB);
    } else if (pbMethod->rpc_->method == DvebRpc::kMK) {
      answerGet = RunMethodK(pbMethod->rpc_, pVEB);
    } else if (pbMethod->rpc_->method == DvebRpc::kMStatus) {
      answerGet = RunMethodStatus(pbMethod->rpc_, pVEB);
    }


    if (NULL != answerGet) {
      writeAnswer(sock, answerGet->answ_);
      ret = true;
    }

  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "unknown exception" << std::endl;
  }

  return ret;
}
