
#ifndef BIGDATA_LOG_SETUP_HH_
#define BIGDATA_LOG_SETUP_HH_

#include <boost/log/trivial.hpp>

namespace bigdata {

namespace log {

extern void setup(boost::log::trivial::severity_level level);

}
}

#endif  // BIGDATA_LOG_SETUP_HH_
