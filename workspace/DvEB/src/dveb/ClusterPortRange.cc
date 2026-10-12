/*
 * ClusterPortRange.cc
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#include "ClusterPortRange.hh"
#include "ProtoVEBClient.hh"

#include <string>
#include "ProtoVEBk.hh"

ClusterPortRange::ClusterPortRange(uint8_t k_parent, const std::string & address, uint16_t port_base) :
k_parent_(k_parent), address_(address), port_base_(port_base), has_hacked_(false), hacked_index_(0) {
}

ClusterPortRange::~ClusterPortRange() {
  // TODO Auto-generated destructor stub
}

const std::shared_ptr<ProtoVEB> ClusterPortRange::operator [](uint64_t index) {
  if (index > ProtoVEB::k_properties[k_parent_].sqrt_high_u) {
    std::cerr << "Out of range" << std::endl;
    return std::shared_ptr<ProtoVEB>();
  }

  if (has_hacked_ && index == hacked_index_) {
    return hacked_pVEB_;
  }

  std::shared_ptr<ProtoVEBClient> p(new ProtoVEBClient(address_, std::to_string(port_base_ + index)));
  return std::dynamic_pointer_cast<ProtoVEB>(p);
}

void ClusterPortRange::set(uint64_t index, std::shared_ptr<ProtoVEB> pVEB) {
  throw std::runtime_error("ClusterPortRange can't be set.");
}

void ClusterPortRange::setHack(uint64_t index, std::shared_ptr<ProtoVEB> pVEB) {
  has_hacked_ = true;
  hacked_index_ = index;
  hacked_pVEB_ = pVEB;
}
