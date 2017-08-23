#!/bin/bash
# This is myscript.sh
while read line; do
   if [[ $line =~ ^[A-Za-z0-9._-]+@([A-Za-z0-9.-]+)$ ]]
   then
      echo "Valid email ${BASH_REMATCH[0]}"
      echo "Domain is ${BASH_REMATCH[1]}"
   else
      echo "Invalid email address!"
   fi
done
