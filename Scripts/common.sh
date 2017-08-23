#!/bin/bash

# References:
#  - https://github.com/mortea15/DAT234/tree/master/MandatoryBashScripting/bash-examples
#  - lorem ipsum...

LINE_SEPERATOR="\n   ################################################################   \n"

function print_header {
	print_separator
	print_prefix_line ${1:-" Result: "}
}

function print_separator {
	echo -e "${LINE_SEPERATOR}"
}

function print_prefix_line {
	echo -e "   - ${1:-\"missing string\"}"
}

function print_array {
	ARR=${1:-""}
	for line in ${ARR[@]}; do
		echo -e -n "   - ${line:-"missing string"}\n"
	done
}

declare -a debug 	# must be declared

debug=( \
    [local]="" \
    [remote]="" \
    [all]="" \
)

declare -a scan_patterns        # must be declared

scan_patterns=( \
	# his will search for X only in variable names and output only matching variable names:
    [search_grep_exact]="set | grep -oP '^\w*{{SEARCH_PATTERN}}\w*(?==)'" \
	# or for easier editing of searched pattern
    [search_grep_wildcard]="set | grep -oP '^\w*(?==)' | grep {{SEARCH_PATTERN}}" \
	# or simply (maybe more easy to remember)
    [search_grep_default]="set | cut -d= -f1 | grep {{SEARCH_PATTERN}}" \
	# If you want to match X inside variable names, but output in name=value form, then:
	[search_grep_inline_key]="set | grep -P '^\w*X\w*(?==)'" \
	# and if you want to match X inside variable names, but output only value, then:
	[search_grep_inline_value]="set | grep -P '^\w*X\w*(?==)' | grep -oP '(?<==).*'" \
)

function autoload {
	AUTOLOAD_GITHUB_REMOTE_URL=$(git config --get remote.origin.url)
	AUTOLOAD_MACHINE_LOCAL_KERNEL="$(uname -sr)"
	AUTOLOAD_MACHINE_LOCAL_INFO="$(uname -a)"
	AUTOLOAD_MACHINE_LOCAL_ARCH="$(uname -m)"
	AUTOLOAD_MACHINE_LOCAL_OS_NAME="$(uname -n)"
	AUTOLOAD_MACHINE_LOCAL_OS_REL="$(uname -r)"
	AUTOLOAD_MACHINE_LOCAL_OS_VERSION="$(uname -v)"
	list_vars_by_prefix print search_grep_inline_key "AUTOLOAD_"
}

function list_vars_by_prefix {
	LVBP_OUTPUT_FORMAT=${1:-"print"} 				# options: print_only, ret_str_debug, ret_str_variable, ret_array
	LVBP_SEARCH_MODE=${2:-"search_grep_default"}  	# options: search_grep_exact, search_grep_wildcard, search_grep_default, search_grep_inline_key, search_grep_inline_value 
	LVBP_SEARCH_PATTERN=${3:-""} 					# options: string (grep regular expressions authorized)
	LVBP_SEARCH_SCOPE=${4:-"local"} 					# options: limit search to local/global/infile env variables.
	LVBP_SEARCH_CMD_TEMPLATE=$scan_patterns[${SEARCH_MODE}]
	LVBP_SEARCH_QUERY="${SEARCH_CMD_TEMPLATE/{{SEARCH_PATTERN}}/$SEARCH_PATTERN}"
	LVBP_DEBUG=$(set | grep -oP '^\w*(?==)' | grep LVBP_*)
	echo ${LVBP_DEBUG}
}

autoload 

# https://github.com/mortea15/DAT234/blob/master/MandatoryBashScripting/bash-examples/assoc-array.bash

# read -r ans
# case $ans in
# yes)
#    echo "yes!"
#    ;;&
# no)
#    echo "no?"
#    ;;
# *)
#    echo "$ans???"
#    ;;
# esac

# if [[ "$#" -ne 2 ]]; then
#    echo "usage: $0 <argument> <argument>"
#    exit 0
# elif [[ "$1" -eq "$2" ]]; then
#    echo "$1 is arithmetic equal to $2"
# else
#    echo "$1 and $2 arithmetic differs"
# fi
# if [[ "$1" == "$2" ]]; then
#    echo "$1 is string equal to $2"
# else
#    echo "$1 and $2 string differs"
# fi
# if [[ -f "$1" ]]; then
#    echo "$1 is also a file!"
# fi