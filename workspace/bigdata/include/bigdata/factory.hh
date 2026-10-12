#ifndef BIGFDATA_FACTORY_H
#define BIGFDATA_FACTORY_H

#include <memory>
#include <vector>

#include "bigdata/itree.hh"
#include "bigdata/id.hh"
#include "bigdata/natural.hh"
#include "bigdata/globals.hh"

namespace bigdata {

class IChannel;
class Server;

class Factory : public std::enable_shared_from_this<Factory> {
public:
    Factory();

    enum TreeType {
        kTreeAuto,
        kTreeVeb
    };
    enum TreeInstanceType {
        kInstanceAuto,
        kInstanceLocal,
        kInstanceClient,
        kInstanceRemote
    };

    std::tuple<TreeType, TreeInstanceType, std::shared_ptr<ITree> >
    create(tree_uuid_t uuid, bitscnt_t universe, std::shared_ptr<const Id> id, std::shared_ptr<Server> server = NULL, TreeType tree_type = kTreeAuto, TreeInstanceType instance_type = kInstanceAuto);

};

}

#endif  // BIGFDATA_GLOBALS_H
