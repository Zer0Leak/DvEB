/*
 * ProtoVEBClient.cc
 *
 *  Created on: Aug 3, 2015
 *      Author: edgard
 */

#include "dveb/ProtoVEBClient.hh"

#include "dveb/ConnectionClientBsd.hh"

#include "ProtoDataRaw.hh"

ProtoVEBClient::ProtoVEBClient(const std::string & address, const std::string & port) :
    address_(address), port_(port) {
}

ProtoVEBClient::~ProtoVEBClient() {
}

std::unique_ptr<DvebRpc::RpcAnswerPtr> ProtoVEBClient::transfer(ConnectionClient * conn, std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call) {
  uint64_t write_size = rpc_call->rpc_->size;
  rpc_call->rpc_->size = be64toh(rpc_call->rpc_->size);
  conn->socket()->write(rpc_call->rpc_, write_size);

  uint64_t answer_size = 0;
  ssize_t read_size;

  // std::tie(read_status, read_size) =  read_all(socket_, (char*)&answer_head, sizeof(AnswerHead));
  read_size = conn->socket()->read_all(&answer_size, sizeof(answer_size));

  if (read_size != sizeof(answer_size)) {
    throw std::runtime_error("header couldn't be read. read_size = " + std::to_string(read_size) + ", from " + conn->socket()->info());
  }

  answer_size = be64toh(answer_size);

  if (answer_size < sizeof(DvebRpc::RpcAnswer)) {
    throw std::runtime_error(std::string("invalid answer_size = ") + std::to_string(answer_size) + ", from " + conn->socket()->info());
  }

  std::unique_ptr<DvebRpc::RpcAnswerPtr> answer(new DvebRpc::RpcAnswerPtr(answer_size - sizeof(DvebRpc::RpcAnswer)));

  read_size = conn->socket()->read_all((uint8_t*)answer->answ_ + sizeof(uint64_t), answer_size - sizeof(answer_size));

  if (read_size != static_cast<ssize_t>(answer_size - sizeof(answer_size))) {
    throw std::runtime_error("data couldn't be read. read_size = " + std::to_string(read_size) + ", from " + conn->socket()->info());
  }

  return std::move(answer);
}

uint8_t ProtoVEBClient::K() {
  ConnectionClientBsd conn;
  conn.connect(address_, std::stoi(port_));

  std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call(new DvebRpc::RpcCallPtr(0));
  rpc_call->rpc_->size = sizeof(DvebRpc::RpcCall);
  rpc_call->rpc_->method = DvebRpc::kMK;
  rpc_call->rpc_->x = 0;

  std::unique_ptr<DvebRpc::RpcAnswerPtr> answer = transfer(&conn, std::move(rpc_call));

  return answer->answ_->k;
}

std::shared_ptr<ProtoData> ProtoVEBClient::get(uint64_t x) {
  ConnectionClientBsd conn;
  conn.connect(address_, std::stoi(port_));

  std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call(new DvebRpc::RpcCallPtr(0));
  rpc_call->rpc_->size = sizeof(DvebRpc::RpcCall);
  rpc_call->rpc_->method = DvebRpc::kMGet;
  rpc_call->rpc_->x = x;

  std::unique_ptr<DvebRpc::RpcAnswerPtr> answer = transfer(&conn, std::move(rpc_call));

  std::shared_ptr<ProtoDataRaw> pds;
  if(answer->answ_->is_valid) {
    pds.reset(new ProtoDataRaw());
    pds->Set(DvebRpc::GetData(answer->answ_), DvebRpc::GetDataSize(answer->answ_));
  }

  return pds;
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> ProtoVEBClient::getMin() {
  ConnectionClientBsd conn;
  conn.connect(address_, std::stoi(port_));

  std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call(new DvebRpc::RpcCallPtr(0));
  rpc_call->rpc_->size = sizeof(DvebRpc::RpcCall);
  rpc_call->rpc_->method = DvebRpc::kMGetMin;
  rpc_call->rpc_->x = 0;

  std::unique_ptr<DvebRpc::RpcAnswerPtr> answer = transfer(&conn, std::move(rpc_call));

  std::shared_ptr<ProtoDataRaw> pds;
  if(answer->answ_->is_valid) {
    pds.reset(new ProtoDataRaw());
    pds->Set(DvebRpc::GetData(answer->answ_), DvebRpc::GetDataSize(answer->answ_));
  }

  return std::make_pair(answer->answ_->x, pds);
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> ProtoVEBClient::getMax() {
  ConnectionClientBsd conn;
  conn.connect(address_, std::stoi(port_));

  std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call(new DvebRpc::RpcCallPtr(0));
  rpc_call->rpc_->size = sizeof(DvebRpc::RpcCall);
  rpc_call->rpc_->method = DvebRpc::kMGetMax;
  rpc_call->rpc_->x = 0;

  std::unique_ptr<DvebRpc::RpcAnswerPtr> answer = transfer(&conn, std::move(rpc_call));

  std::shared_ptr<ProtoDataRaw> pds;
  if(answer->answ_->is_valid) {
    pds.reset(new ProtoDataRaw());
    pds->Set(DvebRpc::GetData(answer->answ_), DvebRpc::GetDataSize(answer->answ_));
  }

  return std::make_pair(answer->answ_->x, pds);
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> ProtoVEBClient::getSuccessor(uint64_t x) {
  ConnectionClientBsd conn;
  conn.connect(address_, std::stoi(port_));

  std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call(new DvebRpc::RpcCallPtr(0));
  rpc_call->rpc_->size = sizeof(DvebRpc::RpcCall);
  rpc_call->rpc_->method = DvebRpc::kMGetSuccessor;
  rpc_call->rpc_->x = x;

  std::unique_ptr<DvebRpc::RpcAnswerPtr> answer = transfer(&conn, std::move(rpc_call));

  std::shared_ptr<ProtoDataRaw> pds;
  if(answer->answ_->is_valid) {
    pds.reset(new ProtoDataRaw());
    pds->Set(DvebRpc::GetData(answer->answ_), DvebRpc::GetDataSize(answer->answ_));
  }

  return std::make_pair(answer->answ_->x, pds);
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> ProtoVEBClient::getPredecessor(uint64_t x) {
  ConnectionClientBsd conn;
  conn.connect(address_, std::stoi(port_));

  std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call(new DvebRpc::RpcCallPtr(0));
  rpc_call->rpc_->size = sizeof(DvebRpc::RpcCall);
  rpc_call->rpc_->method = DvebRpc::kMGetPredecessor;
  rpc_call->rpc_->x = x;

  std::unique_ptr<DvebRpc::RpcAnswerPtr> answer = transfer(&conn, std::move(rpc_call));

  std::shared_ptr<ProtoDataRaw> pds;
  if(answer->answ_->is_valid) {
    pds.reset(new ProtoDataRaw());
    pds->Set(DvebRpc::GetData(answer->answ_), DvebRpc::GetDataSize(answer->answ_));
  }

  return std::make_pair(answer->answ_->x, pds);
}

std::shared_ptr<ProtoData> ProtoVEBClient::insert(uint64_t x, std::shared_ptr<ProtoData> data) {
  ConnectionClientBsd conn;
  conn.connect(address_, std::stoi(port_));

  std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call(new DvebRpc::RpcCallPtr(data->size()));
  rpc_call->rpc_->size = sizeof(DvebRpc::RpcCall) + data->size();
  rpc_call->rpc_->method = DvebRpc::kMInsert;
  rpc_call->rpc_->x = x;
  memcpy(DvebRpc::GetData(rpc_call->rpc_), data->data(), data->size());

  std::unique_ptr<DvebRpc::RpcAnswerPtr> answer = transfer(&conn, std::move(rpc_call));

  std::shared_ptr<ProtoDataRaw> pds;
  if(answer->answ_->is_valid) {
    pds.reset(new ProtoDataRaw());
    pds->Set(DvebRpc::GetData(answer->answ_), DvebRpc::GetDataSize(answer->answ_));
  }

  return std::dynamic_pointer_cast<ProtoData>(pds);
}

std::shared_ptr<ProtoData> ProtoVEBClient::remove(uint64_t x) {
  ConnectionClientBsd conn;
  conn.connect(address_, std::stoi(port_));

  std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call(new DvebRpc::RpcCallPtr(0));
  rpc_call->rpc_->size = sizeof(DvebRpc::RpcCall);
  rpc_call->rpc_->method = DvebRpc::kMRemove;
  rpc_call->rpc_->x = x;

  std::unique_ptr<DvebRpc::RpcAnswerPtr> answer = transfer(&conn, std::move(rpc_call));

  std::shared_ptr<ProtoDataRaw> pds;
  if(answer->answ_->is_valid) {
    pds.reset(new ProtoDataRaw());
    pds->Set(DvebRpc::GetData(answer->answ_), DvebRpc::GetDataSize(answer->answ_));
  }

  return std::dynamic_pointer_cast<ProtoData>(pds);
}

std::string ProtoVEBClient::status() {
  std::string st;
  ConnectionClientBsd conn;
  conn.connect(address_, std::stoi(port_));

  std::unique_ptr<DvebRpc::RpcCallPtr> rpc_call(new DvebRpc::RpcCallPtr(0));
  rpc_call->rpc_->size = sizeof(DvebRpc::RpcCall);
  rpc_call->rpc_->method = DvebRpc::kMStatus;

  std::unique_ptr<DvebRpc::RpcAnswerPtr> answer = transfer(&conn, std::move(rpc_call));

  if(answer->answ_->is_valid) {
    st = (char *) (DvebRpc::GetData(answer->answ_));
  }

  return st;
}
