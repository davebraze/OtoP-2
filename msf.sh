#!/bin/bash
rm -fr $1
if [ -z $2 ]; then
	echo $1 | time ./benchmark > $1.log
else	
	echo $1 | time ./benchmark -iter $2 -per $3 > $1.log 
fi
