/*
 * ClusterArray.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef DVEB_CLUSTERARRAY_HH_
#define DVEB_CLUSTERARRAY_HH_

#include <vector>
#include <memory>

#include "dveb/Cluster.hh"

class ClusterArray : public Cluster {
public:
  ClusterArray(uint64_t size);

  virtual ~ClusterArray();

  virtual const std::shared_ptr<ProtoVEB> operator [](uint64_t index);

  virtual void set(uint64_t index, std::shared_ptr<ProtoVEB> pVEB);

protected:
  virtual void setHack(uint64_t index, std::shared_ptr<ProtoVEB> pVEB);

  std::vector<std::shared_ptr<ProtoVEB>> cluster_;
};

#endif /* DVEB_CLUSTERARRAY_HH_ */
