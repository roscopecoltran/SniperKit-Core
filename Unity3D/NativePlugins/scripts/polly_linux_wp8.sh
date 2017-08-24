#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

## #################################################################
## WP8
## #################################################################

VALID_ARCHS_WP8=(\
	"linux-mingw-w64-cxx98 "
	"linux-mingw-w64 "
	"mingw-c11 "
	"mingw "
)

# linux-mingw-w64-cxx98
# linux-mingw-w64
# mingw-c11
# mingw

## #################################################################
## WP8 / w86 / mingw
## #################################################################

POLLY_TOOLCHAIN="mingw-c11"
WP8_TOOLCHAIN=${POLLY_TOOLCHAIN}
build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=${WP8_TOOLCHAIN} --config Release --clear --reconfig --jobs 4

## #################################################################
## WP8 / w64 / mingw
## #################################################################

POLLY_TOOLCHAIN="linux-mingw-w64"
WP8_TOOLCHAIN=${POLLY_TOOLCHAIN}
build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=${WP8_TOOLCHAIN} --config Release --clear --reconfig --jobs 4


