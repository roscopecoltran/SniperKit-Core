#!/bin/bash
# while.bash

i=0
while read -r line; do
   foo[i]=$line
   ((i++))
done
echo "i is $i, size of foo ${#foo[@]}"
