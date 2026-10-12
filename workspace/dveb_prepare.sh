#!/bin/bash

# git config --global user.name "Edgard Lima"
# git config --global user.email "edgard.lima@gmail.com"

# uname -n

# sudo apt-get install sudo vim g++ cmake libboost-all-dev curl autogen libssl-dev libgmp-dev tmux git net-tools gdb valgrind tcpdump
# EDITOR=vim visudo

# for coap
# sudo apt-get install autoconf  autoconf libcunit1-dev asciidoc pkg-config

# sudo ifconfig eth1 mtu 1500 up
# sudo ifconfig eth1 mtu 9000 up
# ping -M do -s 8972 ifrog

# git pull && make -C ../build_b VERBOSE=1 && make -C ../build_t VERBOSE=1

# git clone git@bitbucket.org:dveb/dveb.git
# git remote set-url origin git@bitbucket.org:dveb/dveb.git

(sudo /etc/init.d/gdm stop ; sudo /etc/init.d/lightdm stop ; sudo /etc/init.d/gdm3 stop ; sudo /etc/init.d/x11-common stop) 2> /dev/null

sudo swapoff -a && sudo sh -c sync && sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'

if [ $? -ne 0 ]; then
  echo "Failed"
else
  free
  echo "Succedded"
fi

# git pull && cd ../build_b && rm -rf * && cmake ../bigdata && cd ../build_t && rm -rf * && cmake ../test && make -C ../build_b VERBOSE=1 && make -C ../build_t VERBOSE=1 && cd ../build && git status

# sudo tcpdump -i any udp -vvv -XX -e -s 64

# sudo sysctl -w net.core.rmem_max=8388608 && sudo sysctl -w net.core.wmem_max=8388608

# sudo sysctl -w net.core.rmem_max=134217728 && sudo sysctl -w net.core.wmem_max=134217728
# sudo sysctl -w net.ipv4.udp_mem='1638400 1638400 1638400'


sudo sysctl -w  net.core.wmem_max=134217728
sudo sysctl -w  net.core.rmem_max=134217728
sudo sysctl -w  net.ipv4.udp_mem=1638400 1638400 1638400
sudo sysctl -w  net.core.somaxconn=4096
sudo sysctl -w  net.core.netdev_max_backlog=262144
sudo sysctl -w  net.core.optmem_max=134217728
sudo sysctl -w  net.ipv4.udp_rmem_min=65535
sudo sysctl -w  net.ipv4.udp_wmem_min=65535


sudo sysctl -w  net.core.wmem_max=212992
sudo sysctl -w  net.core.rmem_max=212992
sudo sysctl -w  net.ipv4.udp_mem=186345 248461 372690
sudo sysctl -w  net.core.somaxconn=128
sudo sysctl -w  net.core.netdev_max_backlog=1000
sudo sysctl -w  net.core.optmem_max=20480
sudo sysctl -w  net.ipv4.udp_rmem_min=4096
sudo sysctl -w  net.ipv4.udp_wmem_min=4096

sudo sysctl net.core.wmem_max
sudo sysctl net.core.rmem_max
sudo sysctl net.ipv4.udp_mem
sudo sysctl net.core.somaxconn
sudo sysctl net.core.netdev_max_backlog
sudo sysctl net.core.optmem_max
sudo sysctl net.ipv4.udp_rmem_min
sudo sysctl net.ipv4.udp_wmem_min



net.ipv4.eth1.rp_filter=0

# for machine in {camaro,ferrari,lamborgine,bugatti,maserati}; do echo $machine; scp node_exp_10.txt ${machine}:~/dveb/workspace/build/ ; done

# awk -F"," 'BEGIN {sum=0;empty=0} {for (i=1; i<=NF; i++) {sum+=$i; print $i; if($i==" "){empty+=1};  }} END {print sum/(NF-empty)}' 

(start=`date`;  (./test lll < root_exp_20.txt 2>&1 | tee out_exp_30.txt) ; var=$(tail out_exp_30.txt)  ; echo "test started at '${start}' and finished '$(date)' with res '$?'\n${var}" | mail -s "test done" edgard.lima@gmail.com )


(confno="32"; testno="32"; start=`date`;  timeres=$((/usr/bin/time -f "'%C' finished in '%E'" /bin/bash -c "./test -- < root_exp_${confno}.txt > out_exp_${testno}.txt 2>&1") 2>&1 ) ; var=$(tail out_exp_${testno}.txt)  ; echo -e "test ${timeres}\n\nStarted at '${start}' and finished at '$(date)' with result:\n\n${var}" | mail -s "test ${testno} done" edgard.lima@gmail.com )




