/*
 * ProtoVEBk1Summary.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef DVEB_PROTOVEBK1SUMMARY_HH_
#define DVEB_PROTOVEBK1SUMMARY_HH_

#include "dveb/ProtoVEB.hh"

class ProtoVEBk1Summary: public ProtoVEB {
public:
  ProtoVEBk1Summary();
  virtual ~ProtoVEBk1Summary();

  virtual uint8_t K() {
    return 0;
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMin();

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMax();

  virtual std::shared_ptr<ProtoData> get(uint64_t x);

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getSuccessor(uint64_t x);

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getPredecessor(uint64_t x);

  virtual std::shared_ptr<ProtoData> insert(uint64_t x, std::shared_ptr<ProtoData> data);

  virtual std::shared_ptr<ProtoData> remove(uint64_t x);

private:
  bool hasmin();
  bool hasmax();
  uint8_t getmin();
  uint8_t getmax();
  void setmin(uint8_t v);
  void setmax(uint8_t v);
  std::shared_ptr<ProtoData> getMinPtr();
  std::shared_ptr<ProtoData> getMaxPtr();

  uint8_t min_max_;
};

#endif /* DVEB_PROTOVEBK1SUMMARY_HH_ */
