#!/bin/bash
# if-num-string.bash

if [[ "$#" -ne 2 ]]; then
   echo "usage: $0 <argument> <argument>"
   exit 0
elif [[ "$1" -eq "$2" ]]; then
   echo "$1 is arithmetic equal to $2"
else
   echo "$1 and $2 arithmetic differs"
fi
if [[ "$1" == "$2" ]]; then
   echo "$1 is string equal to $2"
else
   echo "$1 and $2 string differs"
fi
if [[ -f "$1" ]]; then
   echo "$1 is also a file!"
fi
