#!/bin/bash
# assoc-array.bash

declare -A user        # must be declared
user=(                       \
       [frodeh]="Frode Haug" \
       [ivarm]="Ivar Moe"    \
     )
user+=([lailas]="Laila Skiaker")
echo "${user[ivarm]}"  # print Ivar Moe
echo "${user[@]}"      # print entire array
echo "${#user[@]}"     # length of array
