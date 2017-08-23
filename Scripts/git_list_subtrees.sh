#!/bin/bash

## include common functions
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh


LABEL="Sub-trees available: "
OUTPUT=$(git log | grep git-subtree-dir | tr -d ' ' | cut -d ":" -f2 | sort | uniq)
# print_output "${LABEL}" "${#OUTPUT[@]}"
print_header "${LABEL}"
print_array "${OUTPUT[@]}"

# for line in ${OUTPUT[@]}; do
#	echo -e -n "   - ${line:-"missing string"}\n"
# done
