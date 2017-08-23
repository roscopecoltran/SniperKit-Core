#!/bin/bash
# for.bash

for i in {1..10}; do
   echo -n "$i "
done
echo

# something more useful:

for i in ~/*; do
   if [[ -f $i ]]; then
      echo "$i is a regular file"
   else
      echo "$i is not a regular file"
   fi
done
