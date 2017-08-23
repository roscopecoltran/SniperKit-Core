#!/bin/bash

# U3D_PLUGIN_CMAKE_ROOT=`pwd`
U3D_PLUGIN_CMAKE_ROOT=`pwd`

ANDROID_TOOLCHAIN=android-ndk-r15c-api-19-armeabi-v7a-neon-c11 # android-ndk-r15c-api-19-armeabi-v7a-neon

CONFIG=Debug
HUNTER_CONFIGURATION_TYPES=${CONFIG}
BUILD_QT=ON

PROJECT_BUILD_ARGS=\
(
	"HUNTER_CONFIGURATION_TYPES=Release"
	# "BUILD_QT=${BUILD_QT}"
)

function rename_tab
{
	echo -ne "\033]0;$1:$2\007"
}

function android_ndk_info
{

	ANDROID_NDK=/usr/local/share/android-ndk

	ANDROID_NDK_SOURCE_PROPERTIES="${ANDROID_NDK}/source.properties"
	ANDROID_NDK_VERSION=$(sed -En -e 's/^Pkg.Revision\s*=\s*([0-9a-f]+)/r\1/p' $ANDROID_NDK_SOURCE_PROPERTIES)

	echo
	echo -e "   ANDROID_NDK: ${ANDROID_NDK}"
	echo -e "   ANDROID_NDK_SOURCE_PROPERTIES: ${ANDROID_NDK_SOURCE_PROPERTIES}"
	echo -e "   ANDROID_NDK_VERSION: ${ANDROID_NDK_VERSION}"
	echo 

}

if [ -z "${U3D_PLUGIN_CMAKE_ROOT}" ]; then
    >&2 echo "Must set U3D_PLUGIN_CMAKE_ROOT to top level directory"
    exit
fi

function get_apks
{
    find ${U3D_PLUGIN_CMAKE_ROOT}/_builds/${ANDROID_TOOLCHAIN}-${CONFIG} -name "*.apk" | awk '{print NR " " $1 }'
}

function check_apks 
{
	if [ $# == 0 ]; then
	    get_apks
	else
	    get_apks | awk 'NR=='$1' {print $2}'
	fi
}