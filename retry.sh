#!/bin/sh

trys=1
returnval=1

while [ $returnval -eq 1 ]
do
  echo "try $trys"
  /usr/local/bin/particle flash particlecoreone core_firmware_1459255307823.bin
  returnval=$?
  trys=`expr $trys + 1`
done
