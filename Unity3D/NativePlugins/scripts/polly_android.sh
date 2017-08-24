#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. ${DIR}/common.sh

## #################################################################
## iPhoneOS 
## #################################################################

VALID_ARCHS_IOS=(\
	"android-ndk-r15c-api-19-armeabi-v7a-neon-c11 "
	"android-ndk-r15c-api-19-x86 "
)

## #################################################################
## android-ndk / defaults
## #################################################################

export ANDROID_NDK=/usr/local/share/android-ndk

ANDROID_NDK_SOURCE_PROPERTIES="${ANDROID_NDK}/source.properties"
ANDROID_NDK_VERSION=$(sed -En -e 's/^Pkg.Revision\s*=\s*([0-9a-f]+)/r\1/p' $ANDROID_NDK_SOURCE_PROPERTIES)

echo
echo -e "   ANDROID_NDK: ${ANDROID_NDK}"
echo -e "   ANDROID_NDK_SOURCE_PROPERTIES: ${ANDROID_NDK_SOURCE_PROPERTIES}"
echo -e "   ANDROID_NDK_VERSION: ${ANDROID_NDK_VERSION}"
echo 

## #################################################################
## android-ndk / r15c / api-19 / armeabi-v7a / neon-c11
## #################################################################
POLLY_TOOLCHAIN=android-ndk-r15c-api-19-armeabi-v7a-neon-c11
ANDROID_TOOLCHAIN=${POLLY_TOOLCHAIN}
build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=${ANDROID_TOOLCHAIN} --config ${CMAKE_BUILD_TYPE:-Release} --reconfig --jobs ${CMAKE_JOBS}
check_apks

# legacy:
# mkdir -p ../PluginsBuild/NativePlugins/Android/libs/armeabi-v7a/
# cp build_v7a/libtstunity.so ../PluginsBuild/NativePlugins/Android/libs/armeabi-v7a/libtstunity.so

## #################################################################
## android-ndk / r15c / api-19 / x86
## #################################################################
POLLY_TOOLCHAIN=android-ndk-r15c-api-19-x86
ANDROID_TOOLCHAIN=${POLLY_TOOLCHAIN}
build.py --home=${U3D_PLUGIN_CMAKE_ROOT} --toolchain=${ANDROID_TOOLCHAIN} --config ${CMAKE_BUILD_TYPE:-Release} --reconfig --jobs ${CMAKE_JOBS}
check_apks


# mkdir -p ../PluginsBuild/NativePlugins/Android/libs/x86/
# cp build_x86/libtstunity.so ../PluginsBuild/NativePlugins/Android/libs/x86/libtstunity.so


