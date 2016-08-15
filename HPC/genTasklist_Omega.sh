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
  if [ "$#" -eq 1 ]; then
  	echo "module load Langs/GCC/4.5.3; cd" $2 "; ./msf.sh" $i >> tasklist.txt
  elif [ "$#" -eq 2 ]; then
  	echo "module load Langs/GCC/4.5.3; cd" $2 "; ./msf.sh" $i $3 >> tasklist.txt
  elif [ "$#" -eq 3 ]; then
  	echo "module load Langs/GCC/4.5.3; cd" $2 "; ./msf.sh" $i $3 $4 >> tasklist.txt
  elif [ "$#" -eq 4 ]; then
  	echo "module load Langs/GCC/4.5.3; cd" $2 "; ./msf.sh" $i $3 $4 $5 >> tasklist.txt
  else
  	echo "module load Langs/GCC/4.5.3; cd" $2 "; ./msf.sh" $i $3 $4 $5 $6 >> tasklist.txt
  fi
  i=$((i+1))
done
