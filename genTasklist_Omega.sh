#!/bin/bash
i=1;
while [ $i -le $1 ] 
do 
  if [ -z "$3" ]; then
    echo "module load Langs/GCC/4.5.3; cd" $2 "; ./msf.sh" $i >> tasklist.txt
  else
    echo "module load Langs/GCC/4.5.3; cd" $2 "; ./msf.sh" $i $3 $4 >> tasklist.txt
  fi
  i=$((i+1))
done
