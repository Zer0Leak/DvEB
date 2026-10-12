/*
 * ProtoDataRaw.cc
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#include <cstring>

#include "ProtoDataRaw.hh"

ProtoDataRaw::ProtoDataRaw() {
}

void ProtoDataRaw::Move(std::string & buffer) {
  buffer_ = std::move(buffer);
}

void ProtoDataRaw::Set(const void * buffer, uint64_t size) {
  if (0 == size) {
    buffer_.clear();
  } else {
    buffer_.resize(size);
    memcpy((char *)buffer_.data(), buffer, size);
  }
}

ProtoDataRaw::~ProtoDataRaw() {
}


