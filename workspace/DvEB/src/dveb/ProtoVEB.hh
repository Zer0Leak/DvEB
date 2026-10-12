/*
 * ProtoVEB.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef DVEB_PROTOVEB_HH_
#define DVEB_PROTOVEB_HH_

#include <memory>
#include <cstdint>

#include "dveb/ProtoData.hh"

class ProtoVEB : public ProtoData {  // this inheritance is required because of the summary
public:
  ProtoVEB() {
  }
  virtual ~ProtoVEB() {
  }

  struct KProperties {
    uint64_t max; // u = max+1 // 2 ** k = u
    uint64_t sqrt_high_u;
    uint64_t sqrt_low_u;
    uint32_t low_mask;
  };

  static const KProperties k_properties[];

  virtual uint8_t K() = 0;
  virtual std::shared_ptr<ProtoData> get(uint64_t x) = 0;
  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMin() = 0;
  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMax() = 0;
  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getSuccessor(uint64_t x) = 0;
  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getPredecessor(uint64_t x) = 0;
  virtual std::shared_ptr<ProtoData> insert(uint64_t x, std::shared_ptr<ProtoData> data) = 0;
  virtual std::shared_ptr<ProtoData> remove(uint64_t x) = 0;

  virtual bool isMember(uint64_t x) {
    return NULL != get(x);
  }

  virtual std::string status() {
    return "";
  }

protected:
  virtual void HackSetMax(std::shared_ptr<ProtoVEB> pTree);
  virtual void HackSetMin(std::pair<uint64_t, std::shared_ptr<ProtoData>> min);
  virtual void HackInsertChild(uint64_t high, std::shared_ptr<ProtoVEB> pTree);

  friend class DynamicProtoVEB;
};



#endif /* DVEB_PROTOVEB_HH_ */
