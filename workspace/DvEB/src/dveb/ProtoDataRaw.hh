/*
 * ProtoDataRaw.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef DVEB_PROTODATARAW_HH_
#define DVEB_PROTODATARAW_HH_

#include <string>
#include <memory>
#include <cstdint>

#include "dveb/ProtoData.hh"

class ProtoDataRaw : public ProtoData {
public:
  ProtoDataRaw();
  void Set(const void * buffer, uint64_t size);
  void Move(std::string & buffer);
  virtual ~ProtoDataRaw();
  virtual const void * data() const { return buffer_.data(); }
  virtual uint64_t size() const { return buffer_.size(); }

private:
  std::string buffer_;
};



#endif /* DVEB_PROTODATARAW_HH_ */
