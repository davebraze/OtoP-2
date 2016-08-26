#!/bin/bash
if [ "$#" -eq 0 ]; then
	echo "At least one argument is required!"
	exit 1
fi
if [ "$#" -gt 7 ]; then
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
 	echo $1 | time ./OtoP -iter $2 -rep $3 -samp $4 > $1.log
elif [ "$#" -eq 5 ]; then
	echo $1 | time ./OtoP -iter $2 -rep $3 -samp $4 -met $5 > $1.log 	
else
 	echo $1 | time ./OtoP -iter $2 -rep $3 -samp $4 -met $5 -vthres $6 -> $1.log
fi