
#ifndef BIGDATA_ISERIALIZABLE_HH_
#define BIGDATA_ISERIALIZABLE_HH_

#include <cstdio>
#include <cstddef>

namespace bigdata {

class ISerializable {
public:
    virtual ~ISerializable() = default;
    virtual size_t write(FILE *stream) const = 0;
    virtual size_t read(FILE *stream) = 0;
};

}

#endif  // BIGDATA_ISERIALIZABLE_HH_
