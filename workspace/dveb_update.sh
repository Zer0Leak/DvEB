#!/bin/bash

# uname -n

killall DvEBserveri 2> /dev/null

killall -9 DvEBserver 2> /dev/null

rm -rf DvEB
rm -rf DvEBserver
rm -f DvEB.tgz
rm -f DvEBserver.tgz

(scp 192.168.1.1:./DvEB.tgz . && tar -xzf DvEB.tgz) > /dev/null
(scp 192.168.1.1:./DvEBserver.tgz . && tar -xzf DvEBserver.tgz) > /dev/null

if [ $? -ne 0 ]; then
  exit
fi

rm -rf build
(mkdir build && cd build/ && cmake ../DvEBserver/ && make -j8 && cp ../DvEBserver/DvEBserver.sh . && cd && rm -rf DvEB && rm -rf DvEBserver && rm -f DvEB.tgz && rm -f DvEBserver.tgz) > /dev/null 2>&1


if [ $? -ne 0 ]; then
  echo "Failed"
else
  echo "Succedded"
fi

