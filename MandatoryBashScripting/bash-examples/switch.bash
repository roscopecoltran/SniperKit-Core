#!/bin/bash
# switch.bash

read -r ans
case $ans in
yes)
   echo "yes!"
   ;;&
no)
   echo "no?"
   ;;
*)
   echo "$ans???"
   ;;
esac
