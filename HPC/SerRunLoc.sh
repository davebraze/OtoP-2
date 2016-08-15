#!/bin/bash
if [ "$#" -eq 0 ]; then
	echo "At least one argument is required!"
	exit 1
fi
if [ "$#" -gt 6 ]; then
	echo "Too many arguments!"
	exit 1
fi

i=1;
while [ $i -le $1 ] 
do
  echo "run: " $i
  rm -fr $i
  if [ "$#" -eq 1 ]; then
  	echo $i | time ./benchmark > $i.log
  elif [ "$#" -eq 2 ]; then
  	echo $i | time ./bemchmark -iter $2 > $i.log
  elif [ "$#" -eq 3 ]; then
  	echo $i | time ./benchmark -iter $2 -rep $3 > $i.log
  elif [ "$#" -eq 4 ]; then
  	echo $i | time ./benchmark -iter $2 -rep $3 -met $4 > $i.log
  else
  	echo $i | time ./benchmark -iter $2 -rep $3 -met $4 -vthres $5 -> $i.log
  fi
  i=$((i+1))
done
