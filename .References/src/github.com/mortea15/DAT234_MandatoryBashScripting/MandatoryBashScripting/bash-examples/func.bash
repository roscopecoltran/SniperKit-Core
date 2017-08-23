#!/bin/bash
# func.bash

# declare:
function addfloat {
   echo "$1+$2" | bc
}
# use:
addfloat 5.12 2.56 
