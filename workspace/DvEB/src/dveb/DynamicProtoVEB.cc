/*
 * DynamicProtoVEB.cc
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#include "DynamicProtoVEB.hh"

DynamicProtoVEB::DynamicProtoVEB(uint8_t k) :
    k_(k) {
  bool powerOfTwo = (k > 1) && !(k & (k - 1));
  if (false == powerOfTwo) {
    throw std::runtime_error("DynamicProtoVEB must be power of 2");
  }
  pVEB_ = CreatePVEBFromK(k_);
}
DynamicProtoVEB::~DynamicProtoVEB() {
}

uint8_t DynamicProtoVEB::K() {
  return k_;
}

std::shared_ptr<ProtoData> DynamicProtoVEB::insert(uint64_t x, std::shared_ptr<ProtoData> data) {
  while (x > k_properties[k_].max) {
    std::pair<uint64_t, std::shared_ptr<ProtoData>> min = pVEB_->getMin();
    if (NULL != min.second) {
      pVEB_->remove(min.first);
    }
    std::shared_ptr<ProtoVEB> pVEB = CreatePVEBFromK(k_ << 1);

    pVEB->HackSetMax(pVEB_);
    pVEB->HackInsertChild(0, pVEB_);

    pVEB_.swap(pVEB);

    k_ <<= 1;
    pVEB_->HackSetMin(min);
  }
  return pVEB_->insert(x, data);
}

std::shared_ptr<ProtoData> DynamicProtoVEB::get(uint64_t x) {
  return pVEB_->get(x);
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> DynamicProtoVEB::getMin() {
  return pVEB_->getMin();
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> DynamicProtoVEB::getMax() {
  return pVEB_->getMax();
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> DynamicProtoVEB::getSuccessor(uint64_t x) {
  return pVEB_->getSuccessor(x);
}

std::pair<uint64_t, std::shared_ptr<ProtoData>> DynamicProtoVEB::getPredecessor(uint64_t x) {
  return pVEB_->getPredecessor(x);
}

std::shared_ptr<ProtoData> DynamicProtoVEB::remove(uint64_t x) {
  return pVEB_->remove(x);
}
