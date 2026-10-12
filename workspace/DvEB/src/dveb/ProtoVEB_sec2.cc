/*
 * ProtoVEB.cpp
 *
 *  Created on: Jul 8, 2015
 *      Author: edgard
 */


#ifdef SEC2

#include "ProtoVEB_sec2.hh"

#include <cstdint>

// k | u                    | max | sqrt(u)    | log(u) |
// ------------------------------------------------------
// 0 | 2                    | u-1 |            | 0      |
// 1 | 4                    | u-1 | 2          | 2      |
// 2 | 16                   | u-1 | 4          | 4      |
// 3 | 256                  | u-1 | 16         | 8      |
// 4 | 65536                | u-1 | 256        | 16     |
// 5 | 4294967296           | u-1 | 65536      | 32     |
// 6 | 18446744073709551616 | u-1 | 4294967296 | 64     |

const struct ProtoVEB::KProperties ProtoVEB:: k_properties[] =
    {
        {0,          0,          0,          0  },  // 0
        {3,          2,          0x1,        2  },  // 1
        {15,         4,          0x3,        4  },  // 2
        {UINT8_MAX,  16,         0xF,        8  },  // 3
        {UINT16_MAX, 256,        0xFF,       16 },  // 4
        {UINT32_MAX, 65536,      0xFFFF,     32 },  // 5
        {UINT64_MAX, 4294967296, 0xFFFFFFFF, 64 }   // 6
    };

#endif
