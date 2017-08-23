#!/bin/bash
array[1]= df -H |awk '{ print $1 }'
array[2]= df -H | awk '{ print $8 }'
echo ${array[1]}
echo ${array[2]}

