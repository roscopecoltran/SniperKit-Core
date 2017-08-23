# Copyright (c) 2016, Michele Caini
# Copyright (c) 2015-2017, Ruslan Baratov, Luc Michalski
# All rights reserved.

if(DEFINED POLLY_ANDROID_NDK_R15C_API_21_X86_64_CMAKE_)
  return()
else()
  set(POLLY_ANDROID_NDK_R15C_API_21_X86_64_CMAKE_ 1)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_clear_environment_variables.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_init.cmake")

set(ANDROID_NDK_VERSION "r15c")
set(CMAKE_SYSTEM_VERSION "21")
set(CMAKE_ANDROID_ARCH_ABI "x86_64")

polly_init(
    "Android NDK ${ANDROID_NDK_VERSION} / \
API ${CMAKE_SYSTEM_VERSION} / ${CMAKE_ANDROID_ARCH_ABI} / \
c++11 support"
    "Unix Makefiles"
)

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_common.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/flags/cxx11.cmake") # before toolchain!

include("${CMAKE_CURRENT_LIST_DIR}/os/android.cmake")
