#!/bin/bash

############################################# Author(s)
# ref. https://github.com/hellowod/u3d-plugins-development/tree/master/NativePlugins

############################################# Script

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export ANDROID_NDK=/usr/local/share/android-ndk

mkdir -p build_v7a && cd build_v7a
cmake -DANDROID_ABI=armeabi-v7a -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake -DANDROID_TOOLCHAIN_NAME=arm-linux-androideabi-clang3.6 -DANDROID_NATIVE_API_LEVEL=android-9 ../..
cd ..
cmake --build build_v7a --config Release
mkdir -p ../PluginsBuild/NativePlugins/Android/libs/armeabi-v7a/
cp build_v7a/libtstunity.so ../PluginsBuild/NativePlugins/Android/libs/armeabi-v7a/libtstunity.so

mkdir -p build_x86 && cd build_x86
cmake -DANDROID_ABI=x86 -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake -DANDROID_TOOLCHAIN_NAME=x86-clang3.5 -DANDROID_NATIVE_API_LEVEL=android-9 ../..
cd ..
cmake --build build_x86 --config Release
mkdir -p ../PluginsBuild/NativePlugins/Android/libs/x86/
cp build_x86/libtstunity.so ../PluginsBuild/NativePlugins/Android/libs/x86/libtstunity.so


