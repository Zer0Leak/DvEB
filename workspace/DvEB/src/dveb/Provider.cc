/*
 * Provider.cc
 *
 *  Created on: Aug 5, 2015
 *      Author: edgard
 */

#include "Provider.hh"

#include "Cluster.hh"
#include "ProtoVEB.hh"

#include "ClusterArray.hh"
#include "ClusterPortRange.hh"
#include "ClusterIPv4Range.hh"
#include "ProtoVEBk.hh"
#include "ProtoVEBk1Summary.hh"
#include "ProtoVEBClient.hh"
#include "dveb/globals.hh"

std::shared_ptr<Provider> g_provider_(new Provider());

Provider::Provider() : base_pveb_port_(9090) {
  // TODO Auto-generated constructor stub

}

Provider::~Provider() {
  // TODO Auto-generated destructor stub
}

std::shared_ptr<Cluster> Provider::getCluster(uint8_t k_parent) {
  if (k_parent > 32) {
    if (DVEB_REMOTE_CLUSTER_TYPE == DVEB_REMOTE_CLUSTER_PORT) {
      return std::shared_ptr<Cluster>(new ClusterPortRange(k_parent, "127.0.0.1", 8090));
    } else {
      return std::shared_ptr<Cluster>(new ClusterIPv4Range(k_parent));
    }
  } else {
    return std::shared_ptr<Cluster>(new ClusterArray(ProtoVEB::k_properties[k_parent].sqrt_high_u));
  }
}

std::shared_ptr<ProtoVEB> Provider::getProtoVEB(bool summary, uint8_t k) {
  if (k == 1 && summary) {
    //return CreatePVEBFromK(k);
    return std::shared_ptr<ProtoVEB>(new ProtoVEBk1Summary());
  } else if (k == 32) {
    std::shared_ptr<ProtoVEB> p(new ProtoVEBClient("127.0.0.1", std::to_string(base_pveb_port_++)));
    // I could check if(p->K() == k)
    return p;
  } else {
    return CreatePVEBFromK(k);
  }
}

