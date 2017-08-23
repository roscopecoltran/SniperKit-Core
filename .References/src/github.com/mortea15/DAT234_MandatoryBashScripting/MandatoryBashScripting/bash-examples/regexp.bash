#!/bin/bash
# regexp.bash

while read -r line; do
   if [[ $line =~ \
      ^([A-Za-z0-9._-]+)@([A-Za-z0-9.-]+)$ ]]
   then
      echo "Valid email ${BASH_REMATCH[0]}"
      echo "Username is ${BASH_REMATCH[1]}"
      echo "Domain is ${BASH_REMATCH[2]}"
   else
      echo "Invalid email address!"
   fi
done
