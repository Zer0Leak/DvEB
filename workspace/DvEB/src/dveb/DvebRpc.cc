#include "dveb/DvebRpc.hh"


DvebRpc::RpcCall * DvebRpc::CreateRpcCall(uint64_t data_size) {
  const uint64_t total_size = sizeof(RpcCall) + data_size;
  RpcCall * ret_call = reinterpret_cast<RpcCall *>(new uint8_t[total_size]);
  ret_call->size = total_size;
  ret_call->method = kMUndefined;
  ret_call->x = 0;

  return ret_call;
}

void DvebRpc::DeleteRpcCall(RpcCall * rpc_call) {
  delete [] rpc_call;
}

DvebRpc::RpcAnswer * DvebRpc::CreateRpcAnswer(uint64_t data_size) {
  const uint64_t total_size = sizeof(RpcAnswer) + data_size;
  RpcAnswer * ret_ans = reinterpret_cast<RpcAnswer *>(new uint8_t[total_size]);
  ret_ans->x = 0;
  ret_ans->is_valid = false;
  ret_ans->size = total_size;

  return ret_ans;
}

void DvebRpc::DeleteRpcAnswer(RpcAnswer * rpc_answer) {
  delete [] rpc_answer;
}

uint64_t DvebRpc::GetDataSize(const RpcCall * rpc) {
  return rpc->size - sizeof(RpcCall);
}

uint8_t * DvebRpc::GetData(RpcCall * rpc) {
  uint8_t * ret = reinterpret_cast<uint8_t *>(rpc);
  return ret + sizeof(RpcCall);
}

uint64_t DvebRpc::GetDataSize(const RpcAnswer * answ) {
  return answ->size - sizeof(RpcAnswer);
}

uint8_t * DvebRpc::GetData(RpcAnswer * answ) {
  uint8_t * ret = reinterpret_cast<uint8_t *>(answ);
  return ret + sizeof(RpcAnswer);
}

/*
uint64_t DvebRpc::GetRpcSize(const RpcCall * rpc) {
  uint64_t s = sizeof(rpc->size);
  switch(rpc->method) {
  case kMK:
      break;
  case kMGet:
    break;
  case kMGetMin:
    break;
  case kMGetMax:
    break;
  case kMGetSuccessor:
    break;
  case kMGetPredecessor:
    break;
  case kMInsert:
    break;
  case kMRemove:
    break;
  case kMIsMember:
    break;
  }
}

virtual uint8_t K() = 0;
virtual std::shared_ptr<ProtoData> get(uint64_t x) = 0;
virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMin() = 0;
virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMax() = 0;
virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getSuccessor(uint64_t x) = 0;
virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getPredecessor(uint64_t x) = 0;
virtual std::shared_ptr<ProtoData> insert(uint64_t x, std::shared_ptr<ProtoData> data) = 0;
virtual std::shared_ptr<ProtoData> remove(uint64_t x) = 0;

virtual bool isMember(uint64_t x) {
  return NULL != get(x);
}
*/
