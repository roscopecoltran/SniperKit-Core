cmake_minimum_required (VERSION 3.4.1)

option(WITH_FEATURE_PBF_SUPPORT "Allow import from pbf (requires protobuf and zlib)." OFF)

set(CMAKE_BUILD_TYPE Release FORCE)

set(CORE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../core)

set(BINARY_DIR ${PROJECT_SOURCE_DIR}/build/cmake)

set(MAIN_SOURCE ${CORE_SOURCE_DIR}/src)
set(LIB_SOURCE ${CORE_SOURCE_DIR}/lib)
set(SHARED_SOURCE ${CORE_SOURCE_DIR}/shared)

# initialize boost
find_package(Boost REQUIRED)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions("-DHAS_BOOST")
ENDIF()

add_subdirectory(${CORE_SOURCE_DIR}/src ${BINARY_DIR}/src)
add_subdirectory(${CORE_SOURCE_DIR}/shared ${BINARY_DIR}/shared)
