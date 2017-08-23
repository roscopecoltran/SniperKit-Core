#!/bin/bash
# while-pipe-err.bash

i=0
ls -1 | while read -r line; do
   foo[i]=$line
   ((i++))
done
echo "i is $i, size of foo ${#foo[@]}"
