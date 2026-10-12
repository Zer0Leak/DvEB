/*
 * ProtoVEB.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef BIGDATA_TREE_RPC_MULTICAST_HH_
#define BIGDATA_TREE_RPC_MULTICAST_HH_

#include <utility>
#include <stdexcept>

#include "bigdata/globals.hh"
#include "bigdata/id.hh"
#include "bigdata/tree_rpc.hh"
#include "bigdata/id.hh"
#include "bigdata/ichannel.hh"
#include "bigdata/channel_multicast.hh"
#include "bigdata/transaction.hh"
#include "bigdata/server_multicast.hh"

namespace bigdata {

class exception_peer_not_exist : public std::runtime_error {
public:
    exception_peer_not_exist();
    explicit exception_peer_not_exist(const std::string & what_arg);
    explicit exception_peer_not_exist(const char* what_arg);
    virtual ~exception_peer_not_exist() = default;
};

class exception_peer_timedout : public std::runtime_error {
public:
    exception_peer_timedout();
    explicit exception_peer_timedout(const std::string & what_arg);
    explicit exception_peer_timedout(const char* what_arg);
    virtual ~exception_peer_timedout() = default;
};

class Registry;

class TreeRpcMulticast : public TreeRpc {
public:
    TreeRpcMulticast(bitscnt_t universe, std::shared_ptr<Server> server, tree_uuid_t uuid, std::shared_ptr<const Id> id, bool to_root);
    virtual ~TreeRpcMulticast() = default;
    virtual std::shared_ptr<const Id> id() { return id_; }

    virtual std::tuple<bool, std::shared_ptr<ISerializable>,bool> insert(const Natural & key,
                                                                   std::shared_ptr<ISerializable> value);
    virtual std::tuple<bool, std::shared_ptr<ISerializable>,bool> remove(const Natural & key);
    virtual std::pair<bool, std::shared_ptr<ISerializable>> search(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> successor(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> predecessor(const Natural & key);
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> min() const;
    virtual std::pair<Natural, std::shared_ptr<ISerializable>> max() const;
    virtual bitscnt_t universe();

    virtual void reply(tree_uuid_t uuid,  std::shared_ptr<const AddressInfo> addr_info, Method method, const Transaction & transaction,
                       bool has,            bool had_something, std::shared_ptr<const ISerializable> value,             std::shared_ptr<const statistic::Sample> sample, std::shared_ptr<const Id> id) const;
    virtual void reply(tree_uuid_t uuid,  std::shared_ptr<const AddressInfo> addr_info, Method method, const Transaction & transaction,
                       const Natural & key, std::shared_ptr<const ISerializable> value,             std::shared_ptr<const statistic::Sample> sample);
    virtual void replyu(tree_uuid_t uuid, std::shared_ptr<const AddressInfo> addr_info, Method method, const Transaction & transaction,
                        bitscnt_t universe, std::shared_ptr<const statistic::Sample> sample);

    virtual void create_node(bitscnt_t universe, std::shared_ptr<const Id> id);  // this will replace undelying id

    virtual void veb_expanded(std::shared_ptr<ITree> tree);

    virtual void touch();

    static size_t parse(TreeRpc::Method & method, ServerDispatchArgs & sda, const std::vector<uint8_t> & buffer, bool header_only = false);

private:
    mutable std::shared_ptr<Server> server_;
    tree_uuid_t uuid_;
    std::shared_ptr<const Id> id_;
    bool to_root_;
    mutable Transaction transaction_;
    bitscnt_t universe_;

    std::tuple<bool, Natural, std::shared_ptr<ISerializable>, bitscnt_t, bool>
    run_method(TreeRpc::Method method, TreeRpc::Method answer, const Natural * key = NULL, const ISerializable * value = NULL, const Id * expand_id = NULL, std::shared_ptr<const AddressInfo> unicast_addr = nullptr) const;

    static size_t parse_reply(ServerDispatchArgs & sda, FILE *stream);
    static size_t parse_method(ServerDispatchArgs & sda, FILE *stream);

    size_t process_header(Method method, tree_uuid_t uuid, const Transaction & transaction, bitscnt_t universe) const;

    size_t process(Method method, bool new_trans, const Natural * key = NULL, const ISerializable * value = NULL, const Id * expand_id = NULL, const statistic::Sample * sample = NULL) const;
    size_t process_reply(Method method, const Transaction & transaction, tree_uuid_t uuid, bitscnt_t universe, bool has, bool had_something, const Natural * key, const ISerializable * value, const statistic::Sample * sample = NULL, const Id * id = NULL) const;
};

}

#endif  // BIGDATA_TREE_RPC_MULTICAST_HH_
