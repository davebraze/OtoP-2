#!/bin/bash
i=1;
while [ $i -le $1 ] 
do 
  echo "run: " $i
  rm -fr $i
  if [ -z "$2" ]; then
    echo $i | time ./benchmark > $i.log
  else
    echo $i | time ./benchmark -iter $2 -rep $3 > $i.log
  fi
  i=$((i+1))
done
