#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

## #################################################################
## MacOSX 
## #################################################################

VALID_ARCHS_OSX=(\
	"osx-10-13 "
)

## #################################################################
## MacOSX / 10.13
## #################################################################

POLLY_TOOLCHAIN="osx-10-13"
OSX_TOOLCHAIN=${POLLY_TOOLCHAIN}
build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=osx-10-13 --config Releaase --clear --reconfig --jobs 4

