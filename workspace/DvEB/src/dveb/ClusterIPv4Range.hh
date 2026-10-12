/*
 * ClusterIPv4Range.hh
 *
 *  Created on: Aug 20, 2015
 *      Author: edgard
 */

#ifndef DVEB_CLUSTERIPV4RANGE_HH_
#define DVEB_CLUSTERIPV4RANGE_HH_

#include "dveb/Cluster.hh"

class ClusterIPv4Range : public Cluster {
public:
  ClusterIPv4Range(uint8_t k_parent);

  virtual ~ClusterIPv4Range();

  virtual const std::shared_ptr<ProtoVEB> operator [](uint64_t index);

  virtual void set(uint64_t index, std::shared_ptr<ProtoVEB> pVEB);

  virtual std::string status();

protected:
  virtual void setHack(uint64_t index, std::shared_ptr<ProtoVEB> pVEB);

  uint8_t k_parent_;

private:
  bool has_hacked_;
  uint64_t hacked_index_;
  std::shared_ptr<ProtoVEB> hacked_pVEB_;
};

#endif /* DVEB_CLUSTERIPV4RANGE_HH_ */
