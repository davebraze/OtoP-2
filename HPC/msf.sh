#!/bin/bash
if [ "$#" -lt 1 ]; then
	echo "At least one argument is required!"
	exit 1
fi
if [ "$#" -gt 10 ]; then
	echo "Too many arguments!"
	exit 1
fi

rm -fr $1
if [ "$#" -eq 1 ]; then
	echo $1 | time ./OtoP > $1.log
elif [ "$#" -eq 2 ]; then
  echo $1 | time ./OtoP -iter $2 > $1.log
elif [ "$#" -eq 3 ]; then
 	echo $1 | time ./OtoP -iter $2 -rep $3 > $1.log
elif [ "$#" -eq 4 ]; then
 	echo $1 | time ./OtoP -iter $2 -rep $3 -ptop $4 > $1.log
elif [ "$#" -eq 5 ]; then
 	echo $1 | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 > $1.log
elif [ "$#" -eq 6 ]; then
 	echo $1 | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 -rep_ptop $6 > $1.log
elif [ "$#" -eq 7 ]; then
 	echo $1 | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 -rep_ptop $6 -samp $7 > $1.log
elif [ "$#" -eq 8 ]; then
	echo $1 | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 -rep_ptop $6 -samp $7 -met $8 > $1.log 	
elif [ "$#" -eq 9 ]; then
	echo $1 | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 -rep_ptop $6 -samp $7 -met $8 -vthres $9 > $1.log 	
else
 	echo $1 | time ./OtoP -iter $2 -rep $3 -ptop $4 -iter_ptop $5 -rep_ptop $6 -samp $7 -met $8 -vthres $9 -recvec $10 > $1.log
fi