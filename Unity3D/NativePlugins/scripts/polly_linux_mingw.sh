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
## WP8 / w64 / mingw
## #################################################################

POLLY_TOOLCHAIN="linux-mingw-w64"
MINGW_TOOLCHAIN=${POLLY_TOOLCHAIN}
build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=${MINGW_TOOLCHAIN} --config ${CMAKE_BUILD_TYPE:-Release} --reconfig --jobs ${CMAKE_JOBS}

## #################################################################
## WP8 / w86 / mingw
## #################################################################

# POLLY_TOOLCHAIN="mingw-c11"
# MINGW_TOOLCHAIN=${POLLY_TOOLCHAIN}
# build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=${MINGW_TOOLCHAIN} --config ${CMAKE_BUILD_TYPE:-Release} --reconfig --jobs ${CMAKE_JOBS}

