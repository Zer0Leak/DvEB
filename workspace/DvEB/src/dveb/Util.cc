/*
 * Util.cc
 *
 *  Created on: Jul 10, 2015
 *      Author: edgard
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include "Util.hh"

static uint64_t parseLine(char* line) {
  uint8_t i;
  while (*line < '0' || *line > '9')
    line++;
  for (i = 0; line[i] >= '0' && line[i] <= '9'; i++);
  line[i] = '\0';
  return std::stoull(line);
}

uint64_t getUsedMemory() {
  FILE* file = fopen("/proc/self/status", "r");
  uint64_t result = 0;
  char line[128];

  while (fgets(line, 128, file) != NULL) {
    if (strncmp(line, "VmSize:", sizeof("VmSize:")-1) == 0) {
      result = parseLine(&line[sizeof("VmSize:")-1] );
      break;
    }
  }
  fclose(file);
  return result * 1024;
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

  return std::move(osstream);
}
