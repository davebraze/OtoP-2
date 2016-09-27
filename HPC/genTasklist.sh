#!/bin/bash
if [ "$#" -eq 0 ]; then
	echo "At least one argument is required!"
	exit 1
fi
if [ "$#" -gt 9 ]; then
	echo "Too many arguments!"
	exit 1
fi

i=1;
while [ $i -le $1 ] 
do 
  if [ "$#" -eq 2 ]; then
  	echo "cd" $2 "; ./msf.sh" $i >> tasklist.txt
  elif [ "$#" -eq 3 ]; then
  	echo "cd" $2 "; ./msf.sh" $i $3 >> tasklist.txt
  elif [ "$#" -eq 4 ]; then
  	echo "cd" $2 "; ./msf.sh" $i $3 $4 >> tasklist.txt
  elif [ "$#" -eq 5 ]; then
  	echo "cd" $2 "; ./msf.sh" $i $3 $4 $5 >> tasklist.txt
  elif [ "$#" -eq 6 ]; then
  	echo "cd" $2 "; ./msf.sh" $i $3 $4 $5 $6 >> tasklist.txt
  elif [ "$#" -eq 7 ]; then
  	echo "cd" $2 "; ./msf.sh" $i $3 $4 $5 $6 $7 >> tasklist.txt
  else
  	echo "cd" $2 "; ./msf.sh" $i $3 $4 $5 $6 $7 $8 >> tasklist.txt
  fi
  i=$((i+1))
done
