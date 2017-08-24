#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

## #################################################################
## MacOSX 
## #################################################################

VALID_ARCHS_OSX=(\
	"libcxx "
	"linux-gcc-x64 "
)

## #################################################################
## MacOSX / 10.13
## #################################################################

POLLY_TOOLCHAIN="libcxx"
LIBCXX_TOOLCHAIN=${POLLY_TOOLCHAIN}
build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=${LIBCXX_TOOLCHAIN} --config ${CMAKE_BUILD_TYPE:-Release} --reconfig --jobs ${CMAKE_JOBS}