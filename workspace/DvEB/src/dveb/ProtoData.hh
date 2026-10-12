/*
 * ProtoData.hh
 *
 *  Created on: Aug 4, 2015
 *      Author: edgard
 */

#ifndef DVEB_PROTODATA_HH_
#define DVEB_PROTODATA_HH_


class ProtoData {
public:
  virtual ~ProtoData() {}
  virtual const void * data() const { return NULL; }
  virtual uint64_t size() const { return 0; }
};


#endif /* DVEB_PROTODATA_HH_ */
