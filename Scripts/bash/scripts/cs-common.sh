#!/bin/bash

############################################# Author(s)




############################################# Notes




############################################# References

#  - https://github.com/mortea15/DAT234/tree/master/MandatoryBashScripting/bash-examples
#  - lorem ipsum...

############################################# Script

#function check_config_status {
case "$CS_COMMON" in 
  	True|true|1)    
		CONFIG_LOADED=True
		return
  	;;
  	False|false|0)
		echo -e " *** Load default(s) parameters"
		CONFIG_LOADED=False
  	;;
esac
#	return ${CONFIG_LOADED:-False}
#}

. ${DIR}/bash/scripts/bash/cs-env-lookup.sh

# check_config_status

## local path(s), vcs variables
VCS_ROOT_DIR=`pwd`
CS_COMMON=True

## print decorators
LINE_SEPERATOR="\n   ################################################################   \n"

## Bool condition (ensure)
declare -ir BOOL=(0 1)
readonly false=${BOOL[0]}
readonly true=${BOOL[1]}

function print_header {
	print_separator
	print_prefix_line ${1}
	print_line
}

function print_separator {
	echo -e "${LINE_SEPERATOR}"
}

function print_line {
	echo -e "   ____________________________\n"
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

function check_email {
	local CE_INPUT_STR=${1:-""} 					# options: string (case insensitive)
	local CE_OUTPUT_FORMAT=${2:-"print"} 		# options: print_only, print_all, ret_bool, ret_error
	print_separator
	print_prefix_line "Check email:  \"${CE_INPUT_STR}\""
   	if [[ $CE_INPUT_STR =~ ^[A-Za-z0-9._-]+@([A-Za-z0-9.-]+)$ ]]
   	then
		local CE_VALID=1
		print_prefix_line "Valid email:  \"${BASH_REMATCH[0]}\""
		print_prefix_line "Domain email: \"${BASH_REMATCH[1]}\""
   	else
		local CE_VALID=0
		print_prefix_line "Invalid email address \"${CE_INPUT_STR}\"!"
   	fi
	print_separator
	return ${CE_VALID}
}

check_email "michalski.luc@gmail.com"

# https://github.com/mortea15/DAT234/blob/master/MandatoryBashScripting/bash-examples/assoc-array.bash

function cs_autoclear {
	case "$BASH_AUTO_CLEAR" in 
	  	True|true|1)    
			print_prefix_line "clearing screen buffer..."
			clear
			print_prefix_line "hello sniper...!!!!"
	  	;;
	  	False|false|0|*)  
			print_prefix_line " *** WARNING *** no need to clear the screen"
	  	;;
	esac
}
