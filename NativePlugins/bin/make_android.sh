#!/bin/bash

############################################# Author(s)




############################################# Notes




############################################# Script

# export ANDROID_NDK=/Users/xsj/Tools/android-ndk-r10e
export ANDROID_NDK=/usr/local/share/android-ndk

#For compilers to find this software you may need to set:
#    LDFLAGS:  -L/usr/local/opt/libmpc@0.8/lib
#    CPPFLAGS: -I/usr/local/opt/libmpc@0.8/include

# android-ndk-r10e-api-19-armeabi-v7a-neon

# git subtree add --prefix NativePlugins/External/Protocols https://github.com/roscopecoltran/nng.git sniperkit --squash
# git subtree add --prefix .References/src/github.com/mortea15/DAT234_MandatoryBashScripting https://github.com/mortea15/DAT234.git master --squash

mkdir -p build_v7a && cd build_v7a

# cmake \
#   -DANDROID_TOOLCHAIN_NAME=arm-linux-androideabi-clang \
#   -DANDROID_ABI=armeabi \
#   -DANDROID_STL=c++_static \
#   -DANDROID_NATIVE_API_LEVEL=9
#   

# cmake -DANDROID_ABI=armeabi-v7a -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake -DANDROID_TOOLCHAIN_NAME=arm-linux-androideabi-clang3.6 -DANDROID_NATIVE_API_LEVEL=android-9 ../
# build.py --toolchain=android-ndk-r15c-api1-9-armeabi-v7a-neon-c11 --config Releaase --clear --reconfig --jobs 4
cmake -DANDROID_ABI=armeabi-v7a -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake -DANDROID_TOOLCHAIN_NAME=arm-linux-androideabi-clang -DANDROID_NATIVE_API_LEVEL=android-9 ../
cd ..
cmake --build build_v7a --config Release
mkdir -p ../PluginsBuild/NativePlugins/Android/libs/armeabi-v7a/
cp build_v7a/libtstunity.so ../PluginsBuild/NativePlugins/Android/libs/armeabi-v7a/libtstunity.so

# build.py --toolchain=android-ndk-r15c-api-19-x86 --config Releaase --clear --reconfig --jobs 4

mkdir -p build_x86 && cd build_x86
cmake -DANDROID_ABI=x86 -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake -DANDROID_TOOLCHAIN_NAME=x86-clang -DANDROID_NATIVE_API_LEVEL=android-9 ../
# cmake -DANDROID_ABI=x86 -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake -DANDROID_TOOLCHAIN_NAME=x86-clang3.5 -DANDROID_NATIVE_API_LEVEL=android-9 ../

cd ..
cmake --build build_x86 --config Release
mkdir -p ../PluginsBuild/NativePlugins/Android/libs/x86/
cp build_x86/libtstunity.so ../PluginsBuild/NativePlugins/Android/libs/x86/libtstunity.so


