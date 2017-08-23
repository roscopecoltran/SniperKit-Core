#!/bin/bash
if [ $# -lt 2 ]; then
echo "$0: behöver två argument"
else 
echo "Dina argument vart $*"
fi
exit 1
