#!/bin/bash
# for-commands.bash

i=0
for line in ./*; do
   foo[i]=$line
   ((i++))
done
echo "i is $i, size of foo ${#foo[@]}"
