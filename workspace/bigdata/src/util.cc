/*
 * Util.cc
 *
 *  Created on: Jul 10, 2015
 *      Author: edgard
 */

#include <sys/sysinfo.h>
#include <unistd.h>
#include <errno.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>
#include <stdexcept>

#include "bigdata/util.hh"

namespace bigdata {

static uint64_t parse_line(char* line) {
    uint8_t i;
    while (*line < '0' || *line > '9')
        line++;
    for (i = 0; line[i] >= '0' && line[i] <= '9'; i++);
    line[i] = '\0';
    return std::stoull(line);
}

uint64_t get_proc_memory() {
    FILE* file = fopen("/proc/self/status", "r");
    uint64_t result = 0;
    char line[128];

    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "VmSize:", sizeof("VmSize:")-1) == 0) {
            result = parse_line(&line[sizeof("VmSize:")-1] );
            break;
        }
    }
    fclose(file);
    return result * 1024;
}

uint64_t get_total_memory() {
    return static_cast<uint64_t>(get_phys_pages()) * static_cast<uint64_t>(getpagesize());
}

uint64_t get_available_memory() {
    FILE * meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL) {
        throw std::runtime_error(__FILE__ + std::to_string(__LINE__) + ":Can't open '/proc/meminfo'. Reason '" + std::string(strerror(errno)) + "'.");
    }
    unsigned long long mem_avail = 0;
    unsigned long long mem_free = 0;
    unsigned long long mem_buffers = 0;
    unsigned long long mem_cached = 0;

    u_int8_t mask = 0x00;

    char line[256];
    while(fgets(line, sizeof(line), meminfo))
    {
        if(sscanf(line, "MemAvailable: %llu kB", &mem_avail) == 1)
        {
            // mask = 0x7;
            break;
        }
        if(sscanf(line, "MemFree: %llu kB", &mem_free) == 1)
        {
            mask |= 0x1;
        }
        if(sscanf(line, "Buffers: %llu kB", &mem_buffers) == 1)
        {
            mask |= 0x2;
        }
        if(sscanf(line, "Cached: %llu kB", &mem_cached) == 1)
        {
            mask |= 0x4;
        }
        if (mask == 0x7) {
            mem_avail = mem_free + mem_buffers + mem_cached;
            break;
        }
    }

    fclose(meminfo);

    return mem_avail * 1024ull;
}

uint32_t log2(uint32_t n) {
    uint32_t bits = 0;

    if (n > 0xffff) {
        n >>= 16;
        bits = 0x10;
    }

    if (n > 0xff) {
        n >>= 8;
        bits |= 0x8;
    }

    if (n > 0xf) {
        n >>= 4;
        bits |= 0x4;
    }

    if (n > 0x3) {
        n >>= 2;
        bits |= 0x2;
    }

    if (n > 0x1) {
        bits |= 0x1;
    }

    return bits;
}

/*
std::pair<bool, size_t> read_all(std::shared_ptr<boost::asio::ip::tcp::socket> sock, char * data, size_t max_size) {
  size_t res = 0;
  boost::system::error_code error;

  while(res < max_size) {
    size_t length = boost::asio::read(*sock, boost::asio::buffer(data + res, max_size-res), boost::asio::transfer_all());

    if (error == boost::asio::error::eof) {
      return std::make_pair(false, res);
    } else if (error) {
      throw boost::system::system_error(error); // Some other error.
    }

    if (length <= 0) {
      break;
    }

    res += length;

  }

  return std::make_pair(true, res);
}
*/

#define LOG_HEX_LINE_COUNT 16
#define LOG_HEX_BYTE_DELIM '|'
#define LOG_HEX_EMPTY "-- "
#define LOG_HEX_LINE_DELIM '\n'

std::string Hex(const void *buffer, unsigned size, const char * line_indent) {
    char hex_text[LOG_HEX_LINE_COUNT + 1];
    char hex_char[4];
    std::string osstream;

    for (unsigned i = 0; i < size; ++i) {
        int32_t column = i % LOG_HEX_LINE_COUNT;
        uint8_t ch = (reinterpret_cast<const uint8_t*>(buffer))[i];
        if (column > 0 && 0 == (column & 0x7)) {
            osstream += LOG_HEX_BYTE_DELIM;
        } else if (0 == column) {
            osstream += LOG_HEX_LINE_DELIM;
            osstream += line_indent;
        }
        if (ch >= 32 && ch <= 126) {
            hex_text[column] = ch;
        } else {
            hex_text[column] = '.';
        }
        sprintf(hex_char, "%02X ", ch);
        osstream += hex_char;
        if (size - 1 == i) {
            hex_text[column + 1] = '\0';
            while (column < (LOG_HEX_LINE_COUNT - 1)) {
                ++column;
                if (column > 0 && 0 == (column & 0x7)) {
                    osstream += LOG_HEX_BYTE_DELIM;
                }
                osstream += LOG_HEX_EMPTY;
            }
            osstream += ": ";
            osstream += hex_text;
        } else if ((LOG_HEX_LINE_COUNT - 1) == column) {
            hex_text[column + 1] = '\0';
            osstream += ": ";
            osstream += hex_text;
        }
    }

    return osstream;
}


}
