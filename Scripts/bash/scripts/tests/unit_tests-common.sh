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

function test_bool_cases {
	declare -ir TEST_BOOL_CASE=(0 1) #remember BOOL can't be unset till this shell terminate
	readonly false=${TEST_BOOL_CASE[0]}
	readonly true=${TEST_BOOL_CASE[1]}
	#same as declare -ir false=0 true=1
	((true)) && echo "True"
	((false)) && echo "False"
	((!true)) && echo "Not True"
	((!false)) && echo "Not false"
}