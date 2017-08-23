# Build instructions

## Table of content

- [Build using release artifacts](#build-using-release-artifacts)
- [Build from sources](#build-from-sources)
    - [Build on Android](#build-on-android)
    - [Build on Linux](#build-on-linux)
    - [Build on Mac](#build-on-mac)
    - [Build on Windows](#build-on-windows)

utymap is proven to work on the following platforms:
* **Windows**
* **Linux**
* **Mac**
* **Android**

There are two options for building utymap for Unity.

## Build using release artifacts
Download source code and platform specific archive with binaries from [the latest release](https://github.com/reinterpretcat/utymap/releases) and copy content of it to _unity/demo/Assets/Plugins_. Then just import Unity project from _unity/demo_ folder.

## Build from sources
Core library is written on C++11 and has some dependencies. The following versions are used in development environment:
* **CMake 3.1 or higher.** Some specific flags (CMAKE_CXX_STANDARD, POSITION_INDEPENDENT_CODE) are used.
* **Boost 1.55 or higher.** If you're not considering of running unit tests, then you don't need to build it as utymap uses boost's header only libraries (spirit, lexical_cast, property tree, etc.).
* ***[Optional]*** **Protobuf library and compiler 2.6.1 or other compatible.** Protobuf is used for parsing osm pbf files. Has to be compiled for target platform.
* ***[Optional]*** **zlib 1.2.8 or other compatible.** Used for osm pbf files decompression. Has to be compiled for target platform.

**Note:** Protobuf and zlib are required for importing map data from osm pbf format. If you don't need it, you can skip these dependencies by switching corresponding feature off in _core/CMakeLists.txt_

UtyMap.Unity is written on  C# and has <b>dependency</b> on Unity3D specific dlls which can lead to some compiler errors if you have different Unity3D version. In this case, replace _UnityEditor.dll_ and _UnityEngine.dll_ in /unity/library/packages with appropriate versions of these dlls from your distribution. 

Please follow OS specific instructions below.

### Build on Android

This page describes how to create an Android build for armeabi-v7a architecture using Android Studio with NDK support.

*  Install boost. You don't need to compile boost for target architecture as utymap uses header-only libs (e.g. spirit, lexical_cast) If cmake cannot find boost, specify path explicitly in _android/app/CMakeLists.txt_:

    ```
    set (BOOST_ROOT your_absolute_path_to_boost)
    set (Boost_INCLUDE_DIR ${BOOST_ROOT})
    ```

* Import project from utymap's android folder using Android Studio
* Click Make Project button in IDE

    ![as_make](https://user-images.githubusercontent.com/1611077/27253076-2e7477fa-536d-11e7-9947-ec921d93ccf1.PNG)

* Once library is built, copy it from _android/app/build/intermediates/cmake/debug/obj/armeabi-v7a_ to _Assets/Plugins/Android/libs/armeabi-v7a_.
* Compile UtyMap.Unity if needed using mono or .net framework and copy output library to _Assets/Plugins_
* Import Unity3d project from demo _folder_ using Unity Editor.
*  Replace TouchScript plugin with the version built for Android platform (you can use dll from the latest release)


**Note:** Reconfigure unity specific settings if needed (Player Settings -> Configuration; Write Access = SDCard, Device Filter = ARMv7)

**Note:** Do not forget to remove unused files in _StreamingAssets_ folder as they are embedded to apk.

**Note:** Import from OSM Pbf format is disabled by default as it requires compiling protobuf and zlib for armeabi-v7a. This overcomplicates setup for most users

**Note:** More details about android plugins for Unity can be found here: https://docs.unity3d.com/Manual/PluginsForAndroid.html


### Build on Linux

Library is tested with gcc-4.9 and clang-3.7 on Ubuntu 16.04/12.04. If you have some issues, you may check .travis.yml file for details.

* Install dependencies

    ``` bash
    #update repositories: this is for ubuntu precise distributive
    git clone https://github.com/reinterpretcat/utymap.git
    sudo -E apt-add-repository -y "ppa:kalakris/cmake"
    sudo -E apt-add-repository -y "ppa:george-edison55/precise-backports"
    sudo -E apt-add-repository -y "ppa:ubuntu-toolchain-r/test"
    sudo -E apt-add-repository -y "ppa:boost-latest/ppa"
    sudo -E apt-get -yq update
    #install
    sudo -E apt-get -yq --no-install-suggests --no-install-recommends --force-yes
    install cmake cmake-data zlib1g-dev libprotobuf-dev protobuf-compiler
    libboost1.55-all-dev gcc-4.9 g++-4.9
    ```

* Build core library

    ``` bash
    # ensure that correct compiler is selected
    if [ "$CXX" = "g++" ]; then export CXX="g++-4.9" CC="gcc-4.9"; fi
    cd core
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
    ```

* Copy core library

    ``` bash
    cp shared/libUtyMap.Shared.so ../../unity/Assets/Plugins/libUtyMap.Shared.so

    ```

* Build and copy unity library

    ``` bash
    xbuild /p:Configuration=Release UtyMap.Unity.sln
    cp UtyMap.Unity/bin/Release/UtyMap.Unity.dll ../../unity/Assets/Plugins/
    ```


* Import Unity3d project from demo folder using Unity Editor. If you linked dependencies (protobuf/zlib) dynamically ensure that they are accessible by Unity3D.


### Build on Mac

Tested on 10.11.6

    # get the repository
    git clone https://github.com/reinterpretcat/utymap.git
    cd utymap

    # install the required dependencies (get from http://brew.sh if you don't already have it)
    brew install cmake boost protobuf

    # set up a clean build directory
    mkdir code/build
    cd core/build

    # in ./core/build use cmake to generate an .xcodeproj file
    cmake -G Xcode ..

    # in ./core/build use xcode's command line to build the dylib
    xcodebuild -project UtyMap.xcodeproj -target ALL_BUILD -configuration Release

    # change the libUtyMap.Shared.dylib  to UtyMap.Shared.bundle and more to the project folder
    cp shared/Release/libUtyMap.Shared.dylib ../../unity/demo/Assets/Plugins/UtyMap.Shared.bundle

    # switch te the unity/library directory
    cd ../../unity/library/

    # build with mono's xbuild
    xbuild /p:Configuration=Release UtyMap.Unity.sln

    # copy all the Uty dll files
    cp UtyMap.Unity/bin/Release/Uty*.dll ../../unity/demo/Assets/Plugins/

**NOTE** Instructions are provided by [RMKD](https://github.com/RMKD)

### Build on Windows

Visual Studio 2013 is proven for usage on development environment.

* Download and build/install dependencies.
See https://github.com/reinterpretcat/utymap/wiki#build-from-sources

* Make sure that Cmake can find dependencies.
You can register dependencies in your PATH variable or update main CMakeLists.txt, e.g.:

    ```
    set(BOOST_ROOT c:/_libs/boost_1_59_0)
    set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib)

    set(PROTOBUF_INCLUDE_DIR c:/_libs/protobuf-2.6.1/src)
    set(PROTOBUF_LIBRARY c:/_libs/protobuf-2.6.1/vsprojects/x64/Debug/libprotobuf.lib)
    set(PROTOBUF_PROTOC_EXECUTABLE c:/_libs/protobuf-2.6.1/vsprojects/x64/Debug/protoc.exe)

    set(ZLIB_INCLUDE_DIR c:/_libs/zlib-1.2.8)
    set(ZLIB_LIBRARY c:/_libs/zlib-1.2.8/build/Debug/zlibd.lib)

    ```

* Create Visual Studio project by make:

    ``` shell
    cd core
    mkdir build
    cd build
    <path_to_cmake>\cmake.exe -G "Visual Studio 12 Win64" ..
    ```

* Open created solution and build all projects.

* Copy UtyMap.Shared.dll. This file is build artefact in build/shared directory and should be copied into unity/Assets/Plugins directory. Ensure that zlib.dll (or zlibd.dll) is also there or in your PATH variable.

*  Build UtyMap.Unity solution and copy output dll to Plugins folder.

*  Import Unity3d example project from demo folder.
Launch Unity Editor and open unity folder as existing project.
