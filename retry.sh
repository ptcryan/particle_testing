#!/bin/sh

if [ $# -ne 1 ]
then
  echo "No firmware selected to flash"
  exit 1
fi

trys=1
returnval=1

while [ $returnval -eq 1 ]
do
  echo "try $trys"
  particle flash particlecoreone $1
  returnval=$?
  trys=`expr $trys + 1`
done
