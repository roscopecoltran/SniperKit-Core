#!/bin/bash

. ${DIR}/bash/scripts/cs-common.sh

. ${VCS_ROOT_DIR}/.env

list_vars_by_prefix print search_grep_default "AUTOLOAD_"

# print_header " LUC ${LABEL:-$PATTERN_KEY}"
# print_array "${OUTPUT[@]}"
# print_separator

# pattern_export_rules ${PATTERN_KEY}

#PATTERN_KEY="debug_test_key"
#RES=$(load_pattern() ${PATTERN_KEY})
#print_prefix_line "debug: $RES"

#OUTPUT=$(eval $RES)
#LABEL="check_${PKEY}"

#print_header ${LABEL:-"git_list_subtree"}
#print_array "${OUTPUT[@]}"
#print_separator

#load_pattern debug_test_key
