#!/bin/bash
# while-pipe.bash

i=0
while read -r -d ''; do
   foo[i]=$REPLY
   ((i++))
done < <(find . -maxdepth 1 -print0) 
echo "i is $i, size of foo ${#foo[@]}"
