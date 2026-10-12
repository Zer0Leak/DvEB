/*
 * ProtoVEB.hh
 *
 *  Created on: Jul 8, 2015
 *      Author: edgard
 */

#ifndef PROTOVEB_SEC2_HH_
#define PROTOVEB_SEC2_HH_

#ifdef SEC2

#include <memory>
#include <vector>
#include <cstdint>

#include "Util.hh"

class ProtoData {
public:
  virtual ~ProtoData() {
  }
};

// k | u                    | max | sqrt(u)    | log(u) |
// ------------------------------------------------------
// 0 | 2                    | u-1 |            | 0      |
// 1 | 4                    | u-1 | 2          | 2      |
// 2 | 16                   | u-1 | 4          | 4      |
// 3 | 256                  | u-1 | 16         | 8      |
// 4 | 65536                | u-1 | 256        | 16     |
// 5 | 4294967296           | u-1 | 65536      | 32     |
// 6 | 18446744073709551616 | u-1 | 4294967296 | 64     |

class ProtoVEB : public ProtoData {  // this inheritance is required because of the summary
public:
  ProtoVEB(uint8_t k) :
      k_(k) {
  }
  virtual ~ProtoVEB() {
  }

  struct KProperties {
    uint64_t max;
    uint64_t sqrt_u;
    uint32_t low_mask;
    uint8_t log_u;
  };

  static const KProperties k_properties[];

  virtual std::shared_ptr<ProtoData> Get(uint64_t x) = 0;
  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> GetMin() = 0;
  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> GetSuccessor(uint64_t x) = 0;
  virtual std::shared_ptr<ProtoData> Insert(uint64_t x, std::shared_ptr<ProtoData> data) = 0;
  virtual std::shared_ptr<ProtoData> Delete(uint64_t x) = 0;

  virtual bool IsMember(uint64_t x) {
    return NULL != Get(x);
  }

  uint8_t k_;
};

template<uint8_t k>
class ProtoVEBk : public ProtoVEB {
public:
  ProtoVEBk() : ProtoVEB(k) {
    summary_ = std::make_shared<ProtoVEBk<k - 1>>();
    cluster_.resize(k_properties[k_].sqrt_u, NULL);
  }

  virtual std::shared_ptr<ProtoData> Get(uint64_t x) {
    uint32_t high = x >> (k_properties[k_].log_u >> 1);  // log_u / 2 =  log_u >> 1
    uint32_t low = x & k_properties[k_].low_mask;

    if (NULL != cluster_[high]) {
      return cluster_[high]->Get(low);
    }
    return NULL;
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> GetMin() {
    std::shared_ptr<ProtoData> min_cluster;
    uint64_t min_cluster_index;

    std::tie(min_cluster_index, min_cluster) = summary_->GetMin();

    if (NULL != min_cluster) {
      std::shared_ptr<ProtoData> offset_aux;
      uint64_t offset;

      std::tie(offset, min_cluster) = cluster_[min_cluster_index]->GetMin();

      uint64_t index = (min_cluster_index << (k_properties[k_].log_u >> 1)) | offset;
      return std::make_pair(index, min_cluster);
    }

    return std::make_pair(0, std::shared_ptr<ProtoData>());  // NULL
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> GetSuccessor(uint64_t x) {
    std::shared_ptr<ProtoData> sucessor;
    uint64_t offset;

    uint32_t high = x >> (k_properties[k_].log_u >> 1);  // log_u / 2 =  log_u >> 1
    uint32_t low = x & k_properties[k_].low_mask;

    if (NULL != cluster_[high]) {
      std::tie(offset, sucessor) = cluster_[high]->GetSuccessor(low);
    }

    if (NULL != sucessor) {
      uint64_t index = (high << (k_properties[k_].log_u >> 1)) | offset;
      return std::make_pair(index, sucessor);
    } else {
      std::tie(offset, sucessor) = summary_->GetSuccessor(high);

      if (NULL != sucessor) {
        uint64_t offset2_min;
        std::tie(offset2_min, sucessor) = cluster_[offset]->GetMin();
        uint64_t index = (offset << (k_properties[k_].log_u >> 1)) | offset2_min;
        return std::make_pair(index, sucessor);
      }
    }

    return std::make_pair(0, std::shared_ptr<ProtoData>());  // NULL
  }

  virtual std::shared_ptr<ProtoData> Insert(uint64_t x, std::shared_ptr<ProtoData> data) {
    uint32_t high = x >> (k_properties[k_].log_u >> 1);  // log_u / 2 =  log_u >> 1
    uint32_t low = x & k_properties[k_].low_mask;

    if (NULL == cluster_[high]) {
      cluster_[high] = std::make_shared<ProtoVEBk<k - 1>>();
    }
    std::shared_ptr<ProtoData> aux = cluster_[high]->Insert(low, data);
    summary_->Insert(high, cluster_[high]);

    return aux;
  }

  virtual std::shared_ptr<ProtoData> Delete(uint64_t x) {
    std::shared_ptr<ProtoData> aux;
    uint32_t high = x >> (k_properties[k_].log_u >> 1);  // log_u / 2 =  log_u >> 1
    uint32_t low = x & k_properties[k_].low_mask;

    if (NULL != cluster_[high]) {
      aux = cluster_[high]->Delete(low);
      std::shared_ptr<ProtoData> min;
      std::tie(std::ignore, min) = cluster_[high]->GetMin();
      if (NULL == min) {
        summary_->Delete(high);
      }
    }

    return aux;
  }

  std::shared_ptr<ProtoVEBk<k - 1>> summary_;
  std::vector<std::shared_ptr<ProtoVEBk<k - 1>>>cluster_;
};

template<>
class ProtoVEBk<0> : public ProtoVEB {
public:
  ProtoVEBk() : ProtoVEB(0) {
  }

  virtual std::shared_ptr<ProtoData> Get(uint64_t x) {
    return a_[x];
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> GetMin() {
    if (NULL != a_[0]) {
      return std::make_pair(0, a_[0]);
    }
    return std::make_pair(1, a_[1]);  // if NULL returns NULL anyways
  }

  virtual std::pair<uint64_t, std::shared_ptr<ProtoData>> GetSuccessor(uint64_t x) {
    if (1 == x) {
      return std::make_pair(0, std::shared_ptr<ProtoData>());
    }
    return std::make_pair(1, a_[1]);  // if NULL returns NULL anyways
  }

  virtual std::shared_ptr<ProtoData> Insert(uint64_t x, std::shared_ptr<ProtoData> data) {
    a_[x].swap(data);
    return data;
  }

  virtual std::shared_ptr<ProtoData> Delete(uint64_t x) {
    std::shared_ptr<ProtoData> data;
    a_[x].swap(data);
    return data;
  }

private:
  std::shared_ptr<ProtoData> a_[2];
};

#endif

#endif /* PROTOVEB_SEC2_HH_ */
