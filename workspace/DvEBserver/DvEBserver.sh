#!/bin/sh

a=0
port=8083
count=$2

if [ "$1" = "start" ]; then
   while [ $a -lt $count ]
   do
      ./DvEBserver $port k 32 >> dveb.log & 
      a=`expr $a + 1`
      port=`expr $port + 1`
   done
elif [ "$1" = "stop" ]; then
   killall DvEBserver
elif [ "$1" = "status" ]; then
   ps -aux | grep DvEBserver | grep -v grep | grep -v status
else
  echo "Use start | stop | status"
fi

# ./DvEBserver 8083 k 32 & ./DvEBserver 8084 k 32 & ./DvEBserver 8085 k 32 & ./DvEBserver 8086 k 32 & ./DvEBserver 8087 k 32 & ./DvEBserver 8088 k 32 & ./DvEBserver 8089 k 32 & 

# killall DvEBserver ; tar -xzf workspace.tgz && rm -rf build && mkdir build && cd build/ && cmake ../workspace/DvEBserver/ && make -j8 && cd && rm -rf workspace && rm -f workspace.tgz
