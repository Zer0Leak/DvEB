#ifndef DVEB_RPC_HH
#define DVEB_RPC_HH

#include <cstdint>

class DvebRpc {
public:
  static const uint8_t kMUndefined = 0;
  static const uint8_t kMK = 1;
  static const uint8_t kMGet = 2;
  static const uint8_t kMGetMin = 3;
  static const uint8_t kMGetMax = 4;
  static const uint8_t kMGetSuccessor = 5;
  static const uint8_t kMGetPredecessor = 6;
  static const uint8_t kMInsert = 7;
  static const uint8_t kMRemove = 8;
  static const uint8_t kMIsMember = 9;
  static const uint8_t kMStatus = 9;


  struct RpcCall {
    RpcCall() = delete;
    uint64_t size;
    uint64_t x;
    uint8_t method;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
    // data comes here
  };

  struct RpcAnswer {
    RpcAnswer() = delete;
    uint64_t size;
    union {
      uint8_t k;
      struct {
        uint64_t x;
        bool is_valid;
      };
      struct {
        uint64_t reserved1;
        uint64_t reserved2;
      };
    };
    // data comes here
  };

  class RpcCallPtr{
  public:
    RpcCallPtr(uint64_t data_size) { rpc_ = CreateRpcCall(data_size); }
    ~RpcCallPtr() { DeleteRpcCall(rpc_); }
    void Move(RpcCall * rpc) { DeleteRpcCall(rpc_); rpc_ = rpc; }
    RpcCall * rpc_;
  };

  class RpcAnswerPtr{
  public:
    RpcAnswerPtr(uint64_t data_size) { answ_ = CreateRpcAnswer(data_size); }
    ~RpcAnswerPtr() { DeleteRpcAnswer(answ_); }
    RpcAnswer * answ_;
  };

  static RpcCall * CreateRpcCall(uint64_t data_size);

  static void DeleteRpcCall(RpcCall * rpc_call);

  static RpcAnswer * CreateRpcAnswer(uint64_t data_size);

  static void DeleteRpcAnswer(RpcAnswer * rpc_answer);

  static uint64_t GetDataSize(const RpcCall * rpc);

  static uint8_t * GetData(RpcCall * rpc);

  static uint64_t GetDataSize(const RpcAnswer * answ);

  static uint8_t * GetData(RpcAnswer * answ);
};

#endif  // DVEB_RPC_HH
