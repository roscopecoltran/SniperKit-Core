#!/bin/bash

declare -a patterns_list
patterns_list=()

declare -a patterns_history
patterns_history=()

declare -x scan_patterns=(\
	# his will search for X only in variable names and output only matching variable names:
    [debug_test_key]="LUCCCCIIO" \
	# his will search for X only in variable names and output only matching variable names:
    [search_grep_exact]="set | grep -oP '^\w*{{LVBP_SEARCH_PATTERN}}\w*(?==)'" \
	# or for easier editing of searched pattern
    [search_grep_wildcard]="set | grep -oP '^\w*(?==)' | grep {{LVBP_SEARCH_PATTERN}}" \
	# or simply (maybe more easy to remember)
    [search_grep_default]="set | cut -d= -f1 | grep {{LVBP_SEARCH_PATTERN}}" \
	# If you want to match X inside variable names, but output in name=value form, then:
	[search_grep_inline_key]="set | grep -P '^\w*{{LVBP_SEARCH_PATTERN}}\w*(?==)'" \
	# and if you want to match X inside variable names, but output only value, then:
	[search_grep_inline_value]="set | grep -P '^\w*{{LVBP_SEARCH_PATTERN}}\w*(?==)' | grep -oP '(?<==).*'" \
	# test #1
	[search_grep_debug]="set | echo \"CHATATAAA\" | grep {{LVBP_SEARCH_PATTERN}} | tr -d ' ' | cut -d : -f2 | sort | uniq" \
	# get the list of subtree files declared in this git project.
	[git_list_subtree]="git log | grep git-subtree-dir | tr -d ' ' | cut -d \":\" -f2 | sort | uniq" \
)

# 
# usage: add_pattern_history debug_test_key2
function pattern_add_history {
	local pattern_name=${1}
	local pattern_rules=${2}
	patterns_history+=([$pattern_name]="${pattern_rules[*]}")
	echo "${patterns_history[ivarm]}"  # print Ivar Moe
	echo "${patterns_history[@]}"      # print entire array
	echo "${#patterns_history[@]}"     # length of array
}

function list_vars_by_prefix {

	local LVBP_SEARCH_PATTERN=${1:-"LVBP"} 					# options: string (grep regular expressions authorized)
	local LVBP_SEARCH_MODE=${2:-"search_grep_debug"}  # options: search_grep_exact, search_grep_wildcard, search_grep_default, search_grep_inline_key, search_grep_inline_value 
	local LVBP_SEARCH_SCOPE=${3:-"local"} 				# options: limit search to local/global/file env variables.
	local LVBP_OUTPUT_FORMAT=${4:-"print"} 		# options: print_only, ret_str_debug, ret_str_variable, ret_array

	local LVBP_SEARCH_FILE_DEFAULT="$VCS_ROOT_DIR/.env" 	
	local LVBP_SEARCH_FILE=${5:-"$LVBP_SEARCH_FILE_DEFAULT"} 	# options: limit search to local/global/file env variables.
	local LVBP_SEARCH_CMD_TEMPLATE=${scan_patterns[$LVBP_SEARCH_MODE]}
	local LVBP_SEARCH_QUERY="${LVBP_SEARCH_CMD_TEMPLATE[*]/{{LVBP_SEARCH_PATTERN}}/$LVBP_SEARCH_PATTERN}"
	local LVBP_PATTERNS=$scan_patterns

	. ${LVBP_SEARCH_FILE_DEFAULT}

	print_separator
	print_header "list_vars_by_prefix parameter(s): "

	print_prefix_line "-|_ Search root dir: \"${VCS_ROOT_DIR}\""

	print_prefix_line "-|_ Search env file: \"${LVBP_SEARCH_FILE}\""
	print_prefix_line " |_ Search env file (filepath): \"${ENV_FROM_FILE_PATH}\""
	print_prefix_line " |_ Search env file (default): \"${LVBP_SEARCH_FILE_DEFAULT}\""
	print_prefix_line " |_ Search env file (status): \"${ENV_FROM_FILE_STATUS}\""

	print_prefix_line "-|_ Search pattern: \"${LVBP_SEARCH_PATTERN}\""
	print_prefix_line " |_ Search mode: \"${LVBP_SEARCH_MODE}\""
	print_prefix_line " |_ Search scope: \"${LVBP_SEARCH_SCOPE}\""

	print_separator

	# local LVBP_OUTPUT_ALL=$(set | grep ${LVBP_SEARCH_PATTERN} ${LVBP_SEARCH_FILE} | grep ${LVBP_SEARCH_PATTERN} | tr -d ' ' | cut -d ":" -f2 | sort | uniq)
	local LVBP_OUTPUT=""

	case $LVBP_SEARCH_SCOPE in
	file)
		LVBP_OUTPUT=$(grep ${LVBP_SEARCH_PATTERN} ${LVBP_SEARCH_FILE} | tr -d ' ' | cut -d ":" -f2 | sort | uniq)
    ;;
	local)
		# set | grep AUTOLOAD_
		# set | grep AUTOLOAD_ | tr -d ' ' | cut -d : -f2 | sort | uniq
		# LVBP_OUTPUT="${LVBP_PATTERNS[$LVBP_SEARCH_MODE]}"
		SPKEY="$LVBP_SEARCH_MODE"
		SPKEY_RES=${scan_patterns[$SPKEY]}
		print_prefix_line "debug: $SPKEY_RES"
		LVBP_OUTPUT=$(eval $SPKEY_RES)

		# LVBP_OUTPUT=$(eval "$scan_patterns[$LVBP_SEARCH_MODE]")
		print_prefix_line $LVBP_OUTPUT
		print_array "${LVBP_OUTPUT[@]}"
		echo ${LVBP_OUTPUT[*]}
		echo ${LVBP_OUTPUT[@]}
		# echo "set | grep ${LVBP_SEARCH_PATTERN} | tr -d ' ' | cut -d ":" -f2 | sort | uniq"
		# LVBP_OUTPUT=$(set | grep ${LVBP_SEARCH_PATTERN} | tr -d ' ' | cut -d ":" -f2 | sort | uniq)

    ;;
	global)    
		LVBP_OUTPUT=${LVBP_OUTPUT_ALL}
    ;;
	esac

	case $LVBP_SEARCH_SCOPE in
	print_only|print)
		print_array "${LVBP_OUTPUT[@]}"
    ;;
	ret_str_debug|ret_str_variable)    
		echo ${LVBP_OUTPUT}
    ;;
	esac

}

# 
# usage: add_pattern debug_test_key2
function pattern_add_rule {
	local pattern_name=${1}
	local pattern_rules=${2}
	patterns_list+=([${pattern_name}]="${pattern_rules}")
	echo "${patterns_list[ivarm]}"  # print Ivar Moe
	echo "${patterns_list[@]}"      # print entire array
	echo "${#patterns_list[@]}"     # length of array
}

# 
# usage: load_pattern_by_key debug_test_key
function pattern_export_rules {
	local PATTERN_KEY=${1:-'debug_test_key'}
	local PATTERN_SELECTOR=${2:-'by_key'}
	print_prefix_line "PATTERN_KEY: ${PATTERN_KEY}"
	local PATTERN_RULE=${scan_patterns[$PATTERN_KEY]}
	print_prefix_line "PATTERN_RULE: ${PATTERN_RULE}"
	echo ${PATTERN_RULE}
}
