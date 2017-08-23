#!/bin/bash
# array.bash

os=('linux' 'windows')
os[2]='mac'
echo "${os[1]}"  # print windows
echo "${os[@]}"  # print entire array
echo "${#os[@]}" # length of array
