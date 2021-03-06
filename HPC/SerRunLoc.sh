#!/bin/bash
if [ "$#" -lt 3 ]; then
	echo "At least three arguments are required!"
	exit 1
fi
if [ "$#" -gt 11 ]; then
	echo "Too many arguments!"
	exit 1
fi

i=1;
while [ $i -le $1 ] 
do
  echo "run: " $i
  if [ "$#" -eq 3 ]; then
  	echo $i | time ./OtoP -runmode $2 > $i$3.log
  elif [ "$#" -eq 4 ]; then
  	echo $i | time ./OtoP -runmode $2 -iter $4 > $i$3.log
  elif [ "$#" -eq 5 ]; then
  	echo $i | time ./OtoP -runmode $2 -iter $4 -rep $5 > $i$3.log
  elif [ "$#" -eq 6 ]; then
  	echo $i | time ./OtoP -runmode $2 -iter $4 -rep $5 -iter_ptop $6 > $i$3.log
  elif [ "$#" -eq 7 ]; then
  	echo $i | time ./OtoP -runmode $2 -iter $4 -rep $5 -iter_ptop $6 -rep_ptop $7 > $i$3.log	 
  elif [ "$#" -eq 8 ]; then
  	echo $i | time ./OtoP -runmode $2 -iter $4 -rep $5 -iter_ptop $6 -rep_ptop $7 -samp $8 > $i$3.log
  elif [ "$#" -eq 9 ]; then
  	echo $i | time ./OtoP -runmode $2 -iter $4 -rep $5 -iter_ptop $6 -rep_ptop $7 -samp $8 -met $9 > $i$3.log
  elif [ "$#" -eq 10 ]; then
  	echo $i | time ./OtoP -runmode $2 -iter $4 -rep $5 -iter_ptop $6 -rep_ptop $7 -samp $8 -met $9 -vthres $10 > $i$3.log
  else
  	echo $i | time ./OtoP -runmode $2 -iter $4 -rep $5 -iter_ptop $6 -rep_ptop $7 -samp $8 -met $9 -vthres $10 -recvec $11 > $i$3.log
  fi
  i=$((i+1))
done
