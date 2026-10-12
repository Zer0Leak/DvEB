#!/bin/bash

# ls | grep statis_kRemove_ | sort | awk -F'[_.]' '{print $1" "$2" "$3" "$4}'

rm -f allstat_*.csv

files=`ls | grep "statis_$1_$2_mean" | sort`

for file in $files
do
  method=`echo ${file} | awk -F'[_.]' '{print $2}'`
  typ=`echo ${file} | awk -F'[_.]' '{print $3}'`
  inter=`echo ${file} | awk -F'[_.]' '{print $4}'`
  level=`echo ${file} | awk -F'[_.]' '{print $5}'`

  filename=allstat_${method}.csv

  echo $file
  echo "${method} ${typ} ${inter} $level"

  #echo -n "$level, " >> $filename
  #cat statis_${method}_${level}.csv >> $filename
  #echo >> $filename

done


