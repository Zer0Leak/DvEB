/*
 * Provider.hh
 *
 *  Created on: Aug 5, 2015
 *      Author: edgard
 */

#ifndef DVEB_PROVIDER_HH_
#define DVEB_PROVIDER_HH_

#include <cstdint>
#include <memory>

class Cluster;
class ProtoVEB;

class Provider {
public:
  Provider();
  virtual ~Provider();

  std::shared_ptr<Cluster> getCluster(uint8_t k_parent);

  std::shared_ptr<ProtoVEB> getProtoVEB(bool summary, uint8_t k);
private:
  uint16_t base_pveb_port_;
};

extern std::shared_ptr<Provider> g_provider_;

#endif /* DVEB_PROVIDER_HH_ */
