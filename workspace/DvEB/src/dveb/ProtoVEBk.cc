/*
 * ProtoVEBk.cc
 *
 *  Created on: Aug 5, 2015
 *      Author: edgard
 */

#include "ProtoVEBk.hh"

std::shared_ptr<ProtoVEB> CreatePVEBFromK(uint8_t k) {
  switch (k) {
  case 1:
    return std::shared_ptr<ProtoVEB>(new ProtoVEBk<1>());
  case 2:
    return std::shared_ptr<ProtoVEB>(new ProtoVEBk<2>());
  case 4:
    return std::shared_ptr<ProtoVEB>(new ProtoVEBk<4>());
  case 8:
    return std::shared_ptr<ProtoVEB>(new ProtoVEBk<8>());
  case 16:
    return std::shared_ptr<ProtoVEB>(new ProtoVEBk<16>());
  case 32:
    return std::shared_ptr<ProtoVEB>(new ProtoVEBk<32>());
  case 64:
    return std::shared_ptr<ProtoVEB>(new ProtoVEBk<64>());
  case 128:
    return std::shared_ptr<ProtoVEB>(new ProtoVEBk<128>());
  default:
    return std::shared_ptr<ProtoVEB>();
  }
  return std::shared_ptr<ProtoVEB>();
}


