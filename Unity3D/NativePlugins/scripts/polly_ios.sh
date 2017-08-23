#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

## #################################################################
## iPhoneOS 
## #################################################################

XCODEDIR=$(xcode-select -p)

VALID_ARCHS_IOS=(\
	"ios-11-0 "
	" "
)

## #################################################################
## iPhoneOS / 10.3
## #################################################################

POLLY_TOOLCHAIN=${POLLY_TOOLCHAIN:-"ios-11-0"}
IOS_TOOLCHAIN=${POLLY_TOOLCHAIN}
build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=${IOS_TOOLCHAIN} --config Releaase --clear --reconfig --jobs 4

