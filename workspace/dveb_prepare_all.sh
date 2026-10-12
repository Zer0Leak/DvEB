#!/bin/bash

servers="ferrari lamborgine mustang bugatti maserati camaro"

for s in $servers; do
  echo $s
  scp dveb_prepare.sh $s: > /dev/null && ssh $s ./dveb_prepare.sh
  # ssh $s ./dveb_prepare.sh
  if [ $? -ne 0 ]; then
    echo "Failed"
  fi
  echo ""
done





