#!/bin/bash
if [ "$#" -eq 0 ]; then
	echo "At least one argument is required!"
	exit 1
fi
if [ "$#" -gt 7 ]; then
	echo "Too many arguments!"
	exit 1
fi

i=1;
while [ $i -le $1 ] 
do
  echo "run: " $i
  rm -fr $i
  if [ "$#" -eq 1 ]; then
  	echo $i | time ./OtoP > $i.log
  elif [ "$#" -eq 2 ]; then
  	echo $i | time ./OtoP -iter $2 > $i.log
  elif [ "$#" -eq 3 ]; then
  	echo $i | time ./OtoP -iter $2 -rep $3 > $i.log
  elif [ "$#" -eq 4 ]; then
  	echo $i | time ./OtoP -iter $2 -rep $3 -samp $4 > $i.log
  elif [ "$#" -eq 5 ]; then
  	echo $i | time ./OtoP -iter $2 -rep $3 -samp $4 -met $5 > $i.log
  else
  	echo $i | time ./OtoP -iter $2 -rep $3 -samp $4 -met $5 -vthres $6 -> $i.log
  fi
  i=$((i+1))
done
