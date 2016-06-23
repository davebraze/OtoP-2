#!/bin/bash
i=1;
while [ $i -le $1 ] 
do 
  echo "run: " $i
  rm -fr $i
  echo $i | time ./benchmark -iter $2 -rep $3> $i.log
  i=$((i+1))
done
