# CMake project to install protobuf::protoc from hunter Protobuf package
cmake_minimum_required(VERSION 3.0)

# Emulate HunterGate:
# * https://github.com/hunter-packages/gate
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/HunterGate.cmake)
HunterGate(URL "${HUNTER_HOST_URL}" SHA1 "${HUNTER_HOST_SHA1}")

project(download-serialization-host)

hunter_add_package(CapnProto)
hunter_add_package(Protobuf)
hunter_add_package(flatbuffers)

# download Lua
hunter_add_package(Lua)

execute_process(
    COMMAND "${LUA_ROOT}/bin/lua" -v
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT result EQUAL 0)
  message(FATAL_ERROR "lua failed")
endif()

message("Lua version: ${output}")

