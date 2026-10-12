/*
 * Cluster.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef DVEB_CLUSTER_HH_
#define DVEB_CLUSTER_HH_

#include <memory>
#include <cstdint>

class ProtoVEB;

class Cluster {
public:
  virtual ~Cluster() {}

  virtual const std::shared_ptr<ProtoVEB> operator [](uint64_t index) = 0;

  virtual void set(uint64_t index, std::shared_ptr<ProtoVEB> pVEB) = 0;

  virtual std::string status() {
    return "";
  }

protected:
  virtual void setHack(uint64_t index, std::shared_ptr<ProtoVEB> pVEB) = 0;

  template<uint8_t k> friend class ProtoVEBk;
};

#endif /* DVEB_CLUSTER_HH_ */
