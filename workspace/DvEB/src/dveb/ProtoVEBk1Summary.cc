/*
 * ProtoVEBk1Summary.cc
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#include "ProtoVEBk1Summary.hh"

static std::shared_ptr<ProtoData> staticProtoData(new ProtoData());

extern uint64_t leme_count;

ProtoVEBk1Summary::ProtoVEBk1Summary() :
    ProtoVEB() {
  leme_count++;
  min_max_ = 0xFF;
}

ProtoVEBk1Summary::~ProtoVEBk1Summary() {
}

uint8_t K() {
  return 0;
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> ProtoVEBk1Summary::getMin() {
  return std::make_pair(static_cast<uint64_t>(getmin()), getMinPtr());
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> ProtoVEBk1Summary::getMax() {
  return std::make_pair(static_cast<uint64_t>(getmax()), getMaxPtr());
}

std::shared_ptr<ProtoData> ProtoVEBk1Summary::get(uint64_t x) {
  if (x == getmin()) {
    return getMinPtr();
  } else if (x == getmax()) {
    return getMaxPtr();
  }

  return std::shared_ptr<ProtoData>();
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> ProtoVEBk1Summary::getSuccessor(uint64_t x) {
  if (1 == x) {
    return std::make_pair(0, std::shared_ptr<ProtoData>());
  }
  return std::make_pair(static_cast<uint64_t>(getmax()), getMaxPtr());
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> ProtoVEBk1Summary::getPredecessor(uint64_t x) {
  if (0 == x) {
    return std::make_pair(0, std::shared_ptr<ProtoData>());
  }
  return std::make_pair(static_cast<uint64_t>(getmin()), getMinPtr());
}

std::shared_ptr<ProtoData> ProtoVEBk1Summary::insert(uint64_t x, std::shared_ptr<ProtoData> data) {
  if (false == hasmin()) {
    setmin(x);
    setmax(x);
    return std::shared_ptr<ProtoData>();  // NULL;
  }

  if (x == getmin()) {        // allow replace
    getMinPtr().swap(data);
    if (x == getmax()) {
    }
    return data;
  } else if (x == getmax()) {
    getMaxPtr().swap(data);
    setmax(x);
    return data;
  }

  if (x == 0 && getmin() == 1) {
    getMinPtr().swap(data);
    setmin(x);
  } else { // if (x == 1 && getmax() == 0) {
    getMaxPtr().swap(data);
    setmax(x);
  }

  return std::shared_ptr<ProtoData>();
}

std::shared_ptr<ProtoData> ProtoVEBk1Summary::remove(uint64_t x) {
  std::shared_ptr<ProtoData> ret_data;

  if (x != getmin() && x != getmax()) {
    return ret_data;
  }

  if (getmin() == getmax()) {
    getMinPtr().swap(ret_data);
    setmin(0xF);
    setmax(0xF);
    return ret_data;
  }

  if (0 == x) {
    getMinPtr().swap(ret_data);
    setmin(getmax());
  } else {
    getMaxPtr().swap(ret_data);
    setmax(getmin());
  }

  return ret_data;
}

bool ProtoVEBk1Summary::hasmin() {
  uint8_t min = getmin();
  if (min == 0 || min == 1) {
    return true;
  } else {
    return false;
  }
}
bool ProtoVEBk1Summary::hasmax() {
  uint8_t max = getmax();
  if (max == 0 || max == 1) {
    return true;
  } else {
    return false;
  }
}
uint8_t ProtoVEBk1Summary::getmin() {
  return (min_max_ & 0x0F);
}
uint8_t ProtoVEBk1Summary::getmax() {
  return (min_max_ >> 4);
}
void ProtoVEBk1Summary::setmin(uint8_t v) {
  min_max_ = (min_max_ & 0xF0) | v;
}
void ProtoVEBk1Summary::setmax(uint8_t v) {
  min_max_ = (min_max_ & 0x0F) | (v << 4);
}
std::shared_ptr<ProtoData> ProtoVEBk1Summary::getMinPtr() {
  uint8_t min = getmin();
  if (min == 0 || min == 1) {
    return staticProtoData;
  } else {
    return std::shared_ptr<ProtoData>();
  }
}
std::shared_ptr<ProtoData> ProtoVEBk1Summary::getMaxPtr() {
  uint8_t max = getmax();
  if (max == 0 || max == 1) {
    return staticProtoData;
  } else {
    return std::shared_ptr<ProtoData>();
  }
}

