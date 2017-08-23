cmake_minimum_required (VERSION 3.1)
project ("UtyMap")

option(WITH_FEATURE_PBF_SUPPORT "Allow import from pbf (requires protobuf and zlib)." ON)

set(CMAKE_CXX_STANDARD 11)

set(MAIN_SOURCE ${PROJECT_SOURCE_DIR}/src)
set(LIB_SOURCE ${PROJECT_SOURCE_DIR}/lib)
set(TEST_SOURCE ${PROJECT_SOURCE_DIR}/test)
set(SHARED_SOURCE ${PROJECT_SOURCE_DIR}/shared)

# initialize boost
find_package(Boost REQUIRED)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions("-DHAS_BOOST")
ENDIF()

if(WITH_FEATURE_PBF_SUPPORT)
    #initialize protobuf package
    find_package(Protobuf REQUIRED)
    include_directories(${PROTOBUF_INCLUDE_DIR})

    #initialize zlib
    find_package(ZLIB REQUIRED)
    include_directories(${ZLIB_INCLUDE_DIR})
endif()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(shared)
