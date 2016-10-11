#!/bin/bash
if [ "$#" -lt 1 ]; then
	echo "At least one argument is required!"
	exit 1
fi
if [ "$#" -gt 10 ]; then
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
  	echo $i | time ./OtoP -iter $2 -rep $3 -ptop $4 > $i.log
  elif [ "$#" -eq 5 ]; then
  	echo $i | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 > $i.log
  elif [ "$#" -eq 6 ]; then
  	echo $i | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 -rep_ptop $6 > $i.log	 
  elif [ "$#" -eq 7 ]; then
  	echo $i | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 -rep_ptop $6 -samp $7 > $i.log
  elif [ "$#" -eq 8 ]; then
  	echo $i | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 -rep_ptop $6 -samp $7 -met $8 > $i.log
  elif [ "$#" -eq 9 ]; then
  	echo $i | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 -rep_ptop $6 -samp $7 -met $8 -vthres $9 > $i.log
  else
  	echo $i | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 -rep_ptop $6 -samp $7 -met $8 -vthres $9 -recvec $10 > $i.log
  fi
  i=$((i+1))
done
