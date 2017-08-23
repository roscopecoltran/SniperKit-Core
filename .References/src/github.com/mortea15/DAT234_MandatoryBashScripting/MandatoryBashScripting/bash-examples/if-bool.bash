#!/bin/bash
# if-bool.bash

if [[ 1 -eq 2 && 1 -eq 1 || 1 -eq 1 ]]; then
   echo "And has precedence"
else
   echo "Or has precedence"
fi

# force OR precedence: 

if [[ 1 -eq 2 && (1 -eq 1 || 1 -eq 1) ]]; then
   echo "And has precedence"
else
   echo "Or has precedence"
fi
