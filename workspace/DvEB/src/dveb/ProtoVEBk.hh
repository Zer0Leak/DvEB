/*
 * ProtoVEB.hh
 *
 *  Created on: Jul 9, 2015
 *      Author: edgard
 */

#ifndef PROTOVEBK_HH_
#define PROTOVEBK_HH_


#include <memory>
#include <vector>
#include <cstdint>
#include <type_traits>
#include <typeinfo>

#include <iostream>

#include "dveb/Util.hh"
#include "dveb/ProtoVEB.hh"
#include "dveb/ProtoData.hh"
#include "dveb/ClusterArray.hh"
#include "dveb/ProtoVEBk1Summary.hh"
#include "dveb/Provider.hh"
#include "ProtoDataRaw.hh"

extern uint64_t leme_count2;

extern std::shared_ptr<ProtoVEB> CreatePVEBFromK(uint8_t k);

static std::shared_ptr<ProtoData> workAroundInsertSummary(new ProtoData());

template<uint8_t k>
class ProtoVEBk : public ProtoVEB {
private:

  static const uint8_t k_ = k;

#if 1
  typedef typename std::conditional<k <= 8,
                            std::uint8_t,
                            typename std::conditional<k <= 16,
                            std::uint16_t,
                            typename std::conditional<k <= 32,
                            std::uint32_t,
                            std::uint64_t>::type>::type>::type TypeU;  // use only required space per k-level
#else
  typedef uint64_t TypeU;
#endif

  std::pair<TypeU, std::shared_ptr<ProtoData>> min_;
  std::pair<TypeU, std::shared_ptr<ProtoData>> max_;

#if 1
  typedef typename std::conditional<k_ == 2,
      ProtoVEBk1Summary,
      ProtoVEBk<(k >> 1) + (k & 1)> >::type TypeS;
#else
  typedef ProtoVEBk<(k >> 1) + (k & 1)> TypeS;
#endif

public:

  ProtoVEBk() : ProtoVEB() {
    cluster_ = g_provider_->getCluster(k);
    summary_ = g_provider_->getProtoVEB(true, (k >> 1) + (k & 1));
  }

  virtual uint8_t K() { return k; }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMin() {
    return min_;  // NULL
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMax() {
    return max_;  // NULL
  }

  virtual std::shared_ptr<ProtoData> get(uint64_t x) {
    if (x > k_properties[k].max) {
      std::cerr << "x=" << x << " is bigger than max=" << k_properties[k].max << std::endl;
      return std::shared_ptr<ProtoData>();
    }

    uint64_t high = x >> (k >> 1);  // log_u / 2 =  log_u >> 1
    uint32_t low = x & k_properties[k].low_mask;

    if (x == min_.first ) {
      return min_.second;
    } else if (x == max_.first ) {
      return max_.second;
    }

    if (summary_->get(high) != (*cluster_)[high]) {
      // std::cout << "summary_->Get(high) != (*cluster_)[high]" << std::endl;
      // exit (-1);
    }

    if (NULL != (*cluster_)[high]) {
      return (*cluster_)[high]->get(low);
    } else {
      return std::shared_ptr<ProtoData>();
    }
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getSuccessor(uint64_t x) {
    if (x > k_properties[k].max) {
      std::cerr << "x=" << x << " is bigger than max=" << k_properties[k].max << std::endl;
      return std::make_pair(0, std::shared_ptr<ProtoData>());  // NULL
    }

    uint64_t high = x >> (k >> 1);  // log_u / 2 =  log_u >> 1
    uint32_t low = x & k_properties[k].low_mask;

    if (NULL != min_.second && x < min_.first) {
      return min_;
    }

    if (NULL != (*cluster_)[high]) {
      std::pair<uint64_t, std::shared_ptr<ProtoData>> max_low = (*cluster_)[high]->getMax();
      if (NULL != max_low.second && low < max_low.first) {
        std::pair<uint64_t, std::shared_ptr<ProtoData>> successor = (*cluster_)[high]->getSuccessor(low);
        uint64_t index = (high << (k >> 1)) | successor.first;
        return std::make_pair(index, successor.second);
      }
    }
    std::pair<uint64_t, std::shared_ptr<ProtoData>> successor_cluster = summary_->getSuccessor(high);
    if (NULL != successor_cluster.second) {
      std::pair<uint64_t, std::shared_ptr<ProtoData>> successor = (*cluster_)[successor_cluster.first]->getMin();
      uint64_t index = (successor_cluster.first << (k >> 1)) | successor.first;
      return std::make_pair(index, successor.second);
    }

    return std::make_pair(0, std::shared_ptr<ProtoData>());  // NULL
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getPredecessor(uint64_t x) {
    if (x > k_properties[k].max) {
      std::cerr << "x=" << x << " is bigger than max=" << k_properties[k].max << std::endl;
      return std::make_pair(0, std::shared_ptr<ProtoData>());  // NULL
    }

    uint64_t high = x >> (k >> 1);  // log_u / 2 =  log_u >> 1
    uint32_t low = x & k_properties[k].low_mask;

    if (NULL != max_.second && x > max_.first) {
      return max_;
    }

    if (NULL != (*cluster_)[high]) {
      std::pair<uint64_t, std::shared_ptr<ProtoData>> min_low = (*cluster_)[high]->getMin();
      if (NULL != min_low.second && low > min_low.first) {
        std::pair<uint64_t, std::shared_ptr<ProtoData>> predecessor = (*cluster_)[high]->getPredecessor(low);
        uint64_t index = (high << (k >> 1)) | predecessor.first;
        return std::make_pair(index, predecessor.second);
      }
    }
    std::pair<uint64_t, std::shared_ptr<ProtoData>> predecessor_cluster = summary_->getPredecessor(high);
    if (NULL == predecessor_cluster.second) {
      if (NULL != min_.second && x > min_.first) {
        return min_;
      }
    } else {
      std::pair<uint64_t, std::shared_ptr<ProtoData>> predecessor = (*cluster_)[predecessor_cluster.first]->getMax();
      uint64_t index = (predecessor_cluster.first << (k >> 1)) | predecessor.first;
      return std::make_pair(index, predecessor.second);
    }

    return std::make_pair(0, std::shared_ptr<ProtoData>());  // NULL
  }

  virtual std::shared_ptr<ProtoData> insert(uint64_t x, std::shared_ptr<ProtoData> data) {
    if (x > k_properties[k].max) {
      std::cerr << "x=" << x << " is bigger than max=" << k_properties[k].max << std::endl;
      return std::shared_ptr<ProtoData>();  // NULL;
    }

    uint64_t high;
    uint32_t low;

    if (NULL == min_.second) {
      min_.first = x;
      min_.second = data;
      max_ = min_;
      return std::shared_ptr<ProtoData>();  // NULL;
    }

    if (x == min_.first) {        // allow replace
      min_.first = x;
      min_.second.swap(data);
      if (x == max_.first) {
        max_ = min_;
      }
      return data;
    }

    if (x < min_.first) {
      uint64_t aux = min_.first; min_.first = x; x = aux;
      min_.second.swap(data);
    }

    high = x >> (k >> 1);  // log_u / 2 =  log_u >> 1
    low = x & k_properties[k].low_mask;

    if (NULL == (*cluster_)[high]) {
      cluster_->set(high, g_provider_->getProtoVEB(false, (k >> 1)));
    }

    if (NULL == (*cluster_)[high]->getMin().second) {  // we don't need to update the summary if there is already an element there
      summary_->insert(high, workAroundInsertSummary);  // if this happen, insert bellow will be constant-time (null min)
    }
    std::shared_ptr<ProtoData> ret_data = (*cluster_)[high]->insert(low, data);

    if (x >= max_.first) {
      max_.first = x;
      max_.second = data;
    }

    return ret_data;
  }

  virtual void HackSetMax(std::shared_ptr<ProtoVEB> pTree) {
    max_ = pTree->getMax();
  }

  virtual void HackSetMin(std::pair<uint64_t, std::shared_ptr<ProtoData>> min) {
    min_ = min;
  }

  virtual void HackInsertChild(uint64_t high, std::shared_ptr<ProtoVEB> pTree) {
    if (NULL != pTree->getMin().second) {
      summary_->insert(high, std::dynamic_pointer_cast<ProtoData>(workAroundInsertSummary));
    }
    cluster_->setHack(high, pTree);
  }

  virtual std::shared_ptr<ProtoData> remove(uint64_t x) {
    //uint64_t original_x = x;

    if (x > k_properties[k].max) {
      std::cerr << "x=" << x << " is bigger than max=" << k_properties[k].max << std::endl;
      return std::shared_ptr<ProtoData>();  // NULL;
    }

    std::shared_ptr<ProtoData> ret_data;

    uint64_t high;
    uint32_t low;

    if (min_.first == max_.first) {
      if (x == min_.first) {
        min_.first = 0;
        min_.second.swap(ret_data);
        max_ = min_;
        return ret_data;
      }
      return std::shared_ptr<ProtoData>();
    }

    if (x == min_.first) {
      std::pair<uint64_t, std::shared_ptr<ProtoData>> first_cluster = summary_->getMin();
      std::pair<uint64_t, std::shared_ptr<ProtoData>> first_offset = (*cluster_)[first_cluster.first]->getMin();
      x = (first_cluster.first << (k >> 1)) | first_offset.first;
      ret_data = min_.second;
      min_.first = x;
      min_.second = first_offset.second;
    }
    high = x >> (k >> 1);  // log_u / 2 =  log_u >> 1
    low = x & k_properties[k].low_mask;

    if (NULL != (*cluster_)[high]) {
      if (NULL == ret_data) {
        ret_data = (*cluster_)[high]->remove(low);
      } else {
        (*cluster_)[high]->remove(low);
      }

      if (NULL == (*cluster_)[high]->getMin().second) {
        summary_->remove(high);
        if (x == max_.first) {
          std::pair<uint64_t, std::shared_ptr<ProtoData>> summary_max = summary_->getMax();
          if (NULL == summary_max.second) {
            max_ = min_;
          } else {
            std::pair<uint64_t, std::shared_ptr<ProtoData>> smax_offset = (*cluster_)[summary_max.first]->getMax();
            max_.first = (summary_max.first << (k >> 1)) | smax_offset.first;
            max_.second = smax_offset.second;
          }
        }
      } else if (x == max_.first) {
        std::pair<uint64_t, std::shared_ptr<ProtoData>> max_offset = (*cluster_)[high]->getMax();
        max_.first = (high << (k >> 1)) | max_offset.first;
        max_.second = max_offset.second;
      }
    }

    return ret_data;
  }

  virtual std::string status() {
    std::string st;
    //if (NULL != summary_) {
    //  st += summary_->status();
    //}
    if (NULL != cluster_) {
      st += cluster_->status();
    }
    return std::move(st);
  }

protected:
  std::shared_ptr<ProtoVEB> summary_;
  std::shared_ptr<Cluster> cluster_;
};



template<>
class ProtoVEBk<1> : public ProtoVEB {
public:
  ProtoVEBk() : ProtoVEB() {
    leme_count2++;
  }

  virtual uint8_t K() { return 0; }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMin() {
    return min_;  // NULL
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getMax() {
    return max_;  // NULL
  }

  virtual std::shared_ptr<ProtoData> get(uint64_t x) {
    if (x == min_.first ) {
      return min_.second;
    } else if (x == max_.first ) {
      return max_.second;
    }

    return std::shared_ptr<ProtoData>();
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getSuccessor(uint64_t x) {
    if (1 == x) {
      return std::make_pair(0, std::shared_ptr<ProtoData>());
    }
    return max_;  // if NULL returns NULL anyways
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> getPredecessor(uint64_t x) {
    if (0 == x) {
      return std::make_pair(0, std::shared_ptr<ProtoData>());
    }
    return min_;  // if NULL returns NULL anyways
  }

  virtual std::shared_ptr<ProtoData> insert(uint64_t x, std::shared_ptr<ProtoData> data) {
    if (NULL == min_.second) {
      min_.first = x;
      min_.second = data;
      max_ = min_;
      return std::shared_ptr<ProtoData>();  // NULL;
    }

    if (x == min_.first) {        // allow replace
      min_.first = x;
      min_.second.swap(data);
      if (x == max_.first) {
        max_ = min_;
      }
      return data;
    } else if (x == max_.first) {
      max_.first = x;
      max_.second.swap(data);
      return data;
    }

    if (x < min_.first) {
      min_.first = x;
      min_.second.swap(data);
    } else {  // if (x > max_.first) {
      max_.first = x;
      max_.second.swap(data);
    }

    return std::shared_ptr<ProtoData>();
  }

  virtual std::shared_ptr<ProtoData> remove(uint64_t x) {
    std::shared_ptr<ProtoData> ret_data;

    if (x != min_.first && x != max_.first) {
      return ret_data;
    }

    if (min_.first == max_.first) {
      min_.first = 0;
      min_.second.swap(ret_data);
      max_ = min_;
      return ret_data;
    }

    if (0 == x) {
      ret_data = min_.second;
      min_ = max_;
    } else {
      ret_data = max_.second;
      max_ = min_;
    }

    return ret_data;
  }

protected:
  std::pair<uint8_t, std::shared_ptr<ProtoData>> min_;
  std::pair<uint8_t, std::shared_ptr<ProtoData>> max_;
};


#endif /* PROTOVEB_HH_ */
