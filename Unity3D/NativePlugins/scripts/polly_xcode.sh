#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

## #################################################################
## MacOSX 
## #################################################################

VALID_ARCHS_OSX=(\
	"osx-10-13 "
	"xcode "
)

## #################################################################
## MacOSX / 10.13
## #################################################################

POLLY_TOOLCHAIN="xcode"
XCODE_TOOLCHAIN=${POLLY_TOOLCHAIN}
build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=${XCODE_TOOLCHAIN} --config ${CMAKE_BUILD_TYPE:-Release} --reconfig --jobs ${CMAKE_JOBS}

