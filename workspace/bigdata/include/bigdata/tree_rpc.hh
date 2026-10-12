/*
 * ProtoVEB.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef BIGDATA_TREE_RPC_HH_
#define BIGDATA_TREE_RPC_HH_

#include <utility>
#include <ostream>

#include "bigdata/id.hh"
#include "bigdata/iserializable.hh"
#include "bigdata/itree.hh"
#include "bigdata/transaction.hh"
#include "bigdata/ichannel.hh"
#include "bigdata/globals.hh"

namespace bigdata {

namespace statistic {
    struct Sample;
}

class TreeRpc : public ITree {
public:
    enum Method {
        kInvalid = 0,
        kAllMethods = 1,
        kAllAnswers = 2,
        kAll = 3,
        kAck = 4,
        kAck2_Unused,       // 5
        kAckF,              // 6
        kNack,              // 7

        kMethodBegin,       // -

        kId = kMethodBegin, // 8    // update if u insert before
        kInsert,            // 9
        kRemove,            // 10 0x0a
        kSearch,            // 11 0x0b
        kSuccessor,         // 12 0x0c
        kPredecessor,       // 13 0x0d
        kMin,               // 14 0x0e
        kMax,               // 15 0x0f
        kUniverse,          // 16 0x10
        kVeb_expanded,      // 17 0x11

        kMethodEnd,                // -
        kAnswerBegin = kMethodEnd, // -

        kAnswerId = kAnswerBegin,  // 18 0x12
        kAnswerInsert,             // 19 0x13
        kAnswerRemove,             // 20 0x14
        kAnswerSearch,             // 21 0x15
        kAnswerSuccessor,          // 22 0x16
        kAnswerPredecessor,        // 23 0x17
        kAnswerMin,                // 24 0x18
        kAnswerMax,                // 25 0x19
        kAnswerUniverse,           // 26 0x1a
        kAnswerVeb_expanded,       // 27 0x1b
        kAnswerAvailable,          // 28 0x1c
        kAnswerAlloc,              // 29 0x1d

        kAnswerEnd,                // -

        kAvailable = kAnswerEnd,   // 30 0x1e
        kAlloc,                    // 31 0x1f


        kControlForceStop,         // 32 0x20
        kAnswerAllocCheater,       // 33 0x21

        kEnd
    };

    struct MethodHash {
        template <typename T>
        std::size_t operator()(T t) const
        {
            return static_cast<std::size_t>(t);
        }
    };

    static const std::string & to_string(Method method);

    virtual ~TreeRpc() = default;

    virtual void create_node(bitscnt_t universe, std::shared_ptr<const Id> id) = 0;  // this will replace undelying id

    virtual void veb_expanded(std::shared_ptr<ITree>) = 0;

    virtual void reply(tree_uuid_t uuid, std::shared_ptr<const AddressInfo> addr_info, Method method, const Transaction & transction,
                       bool has, bool had_something, std::shared_ptr<const ISerializable> value, std::shared_ptr<const statistic::Sample> sample, std::shared_ptr<const Id> id) const = 0;
    virtual void reply(tree_uuid_t uuid, std::shared_ptr<const AddressInfo> addr_info, Method method, const Transaction & transction,
                       const Natural & key, std::shared_ptr<const ISerializable> value, std::shared_ptr<const statistic::Sample> sample) = 0;
    virtual void replyu(tree_uuid_t uuid, std::shared_ptr<const AddressInfo> addr_info, Method method, const Transaction & transction,
                        bitscnt_t universe, std::shared_ptr<const statistic::Sample> sample) = 0;
};

extern std::ostream& operator <<(std::ostream& stream, TreeRpc::Method method);

}

#endif  // BIGDATA_TREE_RPC_HH_
