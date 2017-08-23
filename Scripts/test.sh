#!/bin/bash

## include common functions
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

function test_print_array {
	OS=('linux' 'windows')
	OS[2]='mac'

	LABEL="print windows:"
	print_output "${LABEL}" "${os[1]}"
	print_separator

	LABEL="print entire array:"
	print_output "${LABEL}" "${os[@]}"
	print_separator

	LABEL="length of array:"
	print_output "${LABEL}" "${#os[@]}"
	print_separator

	# echo "loop over array:"
	print_separator

}