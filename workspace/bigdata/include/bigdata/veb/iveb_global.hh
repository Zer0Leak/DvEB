/*
 * ProtoVEB.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef BIGDATA_IVEB_GLOBAL_HH_
#define BIGDATA_IVEB_GLOBAL_HH_

#include <memory>
#include "bigdata/itree.hh"

namespace bigdata {

class Server;

namespace veb {

class IVebGlobal : public ITree {
public:
    virtual ~IVebGlobal() = default;

    virtual void expanded(std::shared_ptr<ITree>) = 0;

//protected:
//    friend class VebRoot;
//    friend class bigdata::Server;
};

}
}

#endif  // BIGDATA_IVEB_GLOBAL_HH_
