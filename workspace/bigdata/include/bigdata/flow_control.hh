#ifndef BIGDATA_FLOW_CONTROL_H
#define BIGDATA_FLOW_CONTROL_H

#include <cstdint>

#include "bigdata/natural.hh"

namespace bigdata {

class FlowControl {
public:
    static uint32_t timeout();

    static void timeout_feedback(bool timeout);

    static uint32_t long_timeout(bitscnt_t universe);
};

}

#endif  // BIGDATA_FLOW_CONTROL_H


