#ifndef BIGDATA_GLOBALS_H
#define BIGDATA_GLOBALS_H

#include <cstdint>

// the root has the most significant bit set to 1
#define BIGDATA_IS_ROOT(tree_uuid) (0 == ((tree_uuid >> ((sizeof(tree_uuid)*8)-1)) & 0x1))

#define BIGDATA_SET_ROOT(tree_uuid) ((tree_uuid) |= (1ull << ((sizeof(tree_uuid)*8u)-1u)))

#define BIGDATA_UNSET_ROOT(tree_uuid) ((tree_uuid) &= ~(1ull << ((sizeof(tree_uuid)*8u)-1u)))

namespace bigdata {

typedef uint64_t tree_uuid_t;

// extern const uint32_t kMaxNaturalBits;

}

#endif  // BIGDATA_GLOBALS_H
