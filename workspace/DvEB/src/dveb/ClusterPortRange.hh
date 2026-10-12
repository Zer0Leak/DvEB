/*
 * ClusterPortRange.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef DVEB_CLUSTERPORTRANGE_HH_
#define DVEB_CLUSTERPORTRANGE_HH_

#include <string>

#include "dveb/Cluster.hh"

class ClusterPortRange : public Cluster {
public:
  ClusterPortRange(uint8_t k_parent, const std::string & address, uint16_t port_base);

  virtual ~ClusterPortRange();

  virtual const std::shared_ptr<ProtoVEB> operator [](uint64_t index);

  virtual void set(uint64_t index, std::shared_ptr<ProtoVEB> pVEB);

protected:
  virtual void setHack(uint64_t index, std::shared_ptr<ProtoVEB> pVEB);

  uint8_t k_parent_;
  std::string address_;
  uint16_t port_base_;

private:
  bool has_hacked_;
  uint64_t hacked_index_;
  std::shared_ptr<ProtoVEB> hacked_pVEB_;
};

#endif /* DVEB_CLUSTERPORTRANGE_HH_ */
