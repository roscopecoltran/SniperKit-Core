#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

## #################################################################
## iPhoneOS 
## #################################################################

VALID_ARCHS_IOS=(\
	"ios-10-3 "
)

## #################################################################
## iPhoneOS / 10.3
## #################################################################

POLLY_TOOLCHAIN=${POLLY_TOOLCHAIN:-"ios-10-3"}
IOS_TOOLCHAIN=${POLLY_TOOLCHAIN}
build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=ios-10-3 --config Releaase --clear --reconfig --jobs 4

