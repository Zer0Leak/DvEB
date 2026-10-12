/*
 * DynamicProtoVEB.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef DVEB_DYNAMICPROTOVEB_HH_
#define DVEB_DYNAMICPROTOVEB_HH_

#include "dveb/ProtoVEBk.hh"

class DynamicProtoVEB : public ProtoVEB {
public:
  DynamicProtoVEB(uint8_t k = 16);

  virtual ~DynamicProtoVEB();
  virtual uint8_t K();
  virtual std::shared_ptr<ProtoData> insert(uint64_t x, std::shared_ptr<ProtoData> data);
  virtual std::shared_ptr<ProtoData> get(uint64_t x);

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMin();

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMax();

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getSuccessor(uint64_t x);

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getPredecessor(uint64_t x);

  virtual std::shared_ptr<ProtoData> remove(uint64_t x);

  virtual std::string status() {
    if (NULL != pVEB_) {
      return pVEB_->status();
    }
    return "";
  }

protected:
  uint8_t k_;
  std::shared_ptr<ProtoVEB> pVEB_;
};



#endif /* DVEB_DYNAMICPROTOVEB_HH_ */
