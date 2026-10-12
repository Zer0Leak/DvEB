/*
 * ClusterArray.cc
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#include "ClusterArray.hh"

#include "ProtoVEBk.hh"

ClusterArray::ClusterArray(uint64_t size) {
  cluster_.resize(size, NULL);
}

ClusterArray::~ClusterArray() {
}

const std::shared_ptr<ProtoVEB> ClusterArray::operator [](uint64_t index) {
  return cluster_[index];
}

void ClusterArray::set(uint64_t index, std::shared_ptr<ProtoVEB> pVEB) {
  cluster_[index] = pVEB;
}

void ClusterArray::setHack(uint64_t index, std::shared_ptr<ProtoVEB> pVEB) {
  cluster_[index] = pVEB;
}

