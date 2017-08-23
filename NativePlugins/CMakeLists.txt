cmake_minimum_required(VERSION 3.3)


#########################
### CMAKE_MODULE_PATH ###
#########################

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

############################
### HunterGate and cache ###
############################

include(set_hunter_version)
set_hunter_version(APP_HUNTER_GATE_URL APP_HUNTER_GATE_SHA1)

include("cmake/HunterGate.cmake")
HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.19.71.tar.gz"
  SHA1 "8fe144d9490501d398777532e0c0f56bc7bb0ec7"
  FILEPATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/config.cmake"
)
# LOCAL

##########################
### CI Travis/Appveyor ###
##########################

string(COMPARE NOTEQUAL "$ENV{TRAVIS_TAG}" "" travis_deploy)
string(COMPARE EQUAL "$ENV{APPVEYOR_REPO_TAG}" "true" appveyor_deploy)

if(travis_deploy)
  set(version "$ENV{TRAVIS_TAG}")
elseif(appveyor_deploy)
  set(version "$ENV{APPVEYOR_REPO_TAG_NAME}")
else()
  set(version "v0.8.3") # note: rc
endif()

string(REGEX REPLACE "^v" "" version "${version}")

project(tstunity VERSION ${version})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(is_android ${ANDROID}) # syntax compatibility
string(COMPARE EQUAL "${CMAKE_OSX_SYSROOT}" "iphoneos" is_ios)
string(COMPARE EQUAL "${CMAKE_SYSTEM_NAME}" "Linux" is_linux)
string(COMPARE EQUAL "${CMAKE_GENERATOR}" "Xcode" is_xcode)

if(is_xcode)
  option(PROJECT_DISABLE_DSYM "Disable generation of dSYM information" ON)
  if(PROJECT_DISABLE_DSYM)
    set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf")
  endif()
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release;MinSizeRel;RelWithDebInfo" CACHE STRING "Configuration types")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE NO)
endif()

if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
  set(CMAKE_POLICY_DEFAULT_CMP0069 NEW) # for 3rd parties added by add_subdirectory
  cmake_policy(SET CMP0069 NEW)
endif()

hunter_add_package(Sugar)
include(${SUGAR_ROOT}/cmake/Sugar)

include(CheckFunctionExists)
include(CheckStructHasMember)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)

set(export_platform iOS)
set(export_prefix_path ../PluginsBuild/NativePlugins) # /iOS)
set(export_dir ${export_prefix_path}/${export_platform})

function(hunter_export_target target_id interface_namespace export_dir)

  hunter_test_string_not_empty("${target_id}")
  hunter_test_string_not_empty("${interface_namespace}")
  hunter_test_string_not_empty("${export_dir}")

  foreach(queued_library ${queued_libraries})
    message(STATUS "queued_library: ${queued_library}")  
  endforeach()

  message(STATUS "export_dir: ${export_dir}")  
  message(STATUS "interface_namespace: ${interface_namespace}")


  # https://github.com/ltouroumov/cld-net-rogue/blob/d00e83bb9f2c0997836c15da43a6b9a61cea9055/server/CMakeLists.txt#L29-L38
  # https://cmake.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file

  foreach(interface_id ${interface_namespace})

    message(STATUS "interface_id: ${interface_id}")
    if(TARGET ${interface_namespace})

      set(target_file $<TARGET_FILE:${interface_namespace}>)

      add_custom_command(
        TARGET ${target_id}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${export_dir}
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy ${target_file} "${export_dir}"
        VERBATIM
        COMMENT "${target_id} deps"
      )

      message(STATUS " !!!! Supposed to be a valid target to export !!!! ")

    else()

      message(STATUS " !!!! Not a valid target to export !!!! ")
      message(STATUS " export_dir: ${export_dir}")
      message(STATUS " target_file: ${target_file}")

    endif()

    # if $<CONFIG:Release>==1 (

  endforeach()

endfunction()

function(make_binary_resource)
  set(opts GZIP)
  set(val_args INPUT ARRAY_NAME OUTPUT)
  set(mval_args "")
  cmake_parse_arguments(BINRES "${opts}" "${val_args}" "${mval_args}" ${ARGN})


  if (NOT BINRES_INPUT)
    message(FATAL_ERROR "make_binary_resource: no input specified")
  endif()

  if (NOT BINRES_OUTPUT AND NOT BINRES_ARRAY_NAME)
    get_filename_component(in_basename "${BINRES_INPUT}" NAME_WE)
    set(BINRES_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${in_basename}.c")
  endif()

  if (BINRES_OUTPUT AND NOT BINRES_ARRAY_NAME)
    get_filename_component(filename "${BINRES_OUTPUT}" NAME_WE)
    string(MAKE_C_IDENTIFIER ${filename} BINRES_ARRAY_NAME)
  elseif(NOT BINRES_OUTPUT AND BINRES_ARRAY_NAME)
    set(BINRES_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${BINRES_ARRAY_NAME}.c")
  else()
    message(FATAL_ERROR "make_binary_resource: Unable to deduce output path and array name")
  endif()

  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${BINRES_OUTPUT}")

  add_custom_command(
    OUTPUT "${BINRES_OUTPUT}"
    COMMAND $<TARGET_FILE:bin2c>
      -i "${BINRES_INPUT}"
      -a ${BINRES_ARRAY_NAME}
      -l 15
      -o ${BINRES_OUTPUT}
      DEPENDS bin2c ${BINRES_INPUT}
    COMMENT "Making binary resource for ${BINRES_ARRAY_NAME}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
endfunction()

#################
### custom libs #
#################

## #################################################################
## Dependencies - Qt
## #################################################################

## Needs:
#-Wno-error=unused-command-line-argument-hard-error-in-future

option(BUILD_QT "Build with Qt" OFF)

if(BUILD_QT)

  # Some background: https://github.com/ruslo/hunter/issues/253
  hunter_add_package(Qt COMPONENTS qtmultimedia qtdeclarative)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Multimedia REQUIRED)
  find_package(Qt5MultimediaWidgets REQUIRED)
  find_package(Qt5OpenGL REQUIRED)
  find_package(Qt5Qml REQUIRED)
  find_package(Qt5Quick REQUIRED)
  find_package(Qt5QuickTest REQUIRED)
  find_package(Qt5QuickWidgets REQUIRED)

  message("Used Qt:")
  message("  ${QT_ROOT} (root from Hunter)")
  message("  ${Qt5Widgets_DIR} (find_package result)")
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTOUIC ON)
  set(CMAKE_AUTORCC ON)
endif()

# Compatibility Mode introduced by protobuf
# * see examples/Protobuf-legacy for usage of protobuf_MODULE_COMPATIBLE=ON
option(protobuf_MODULE_COMPATIBLE "use protobuf in module compatible mode" OFF)

# If we cross compile for Android or iOS build a separate protoc executable on host to compile .proto files in CMake
if(IOS OR ANDROID)

  # add cmake/host subdiretcory as host project to install protoc
  include(hunter_experimental_add_host_project)
  hunter_experimental_add_host_project(cmake/host)

  add_executable(protobuf::protoc IMPORTED)
  set_property(TARGET protobuf::protoc APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
  set_target_properties(protobuf::protoc PROPERTIES IMPORTED_LOCATION_RELEASE "${HUNTER_HOST_ROOT}/bin/protoc")

  message(STATUS "Using imported protoc from host: ${HUNTER_HOST_ROOT}/bin/protoc")

endif(IOS OR ANDROID)

hunter_add_package(Protobuf)
find_package(Protobuf CONFIG REQUIRED)

hunter_add_package(flatbuffers)
find_package(flatbuffers CONFIG REQUIRED)

# https://github.com/headupinclouds/hunter_eos_example/blob/master/CMakeLists.txt
hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)

# hunter_add_package(CURL)
# find_package(CURL CONFIG REQUIRED)

# hunter_add_package(eos)
# find_package(eos CONFIG REQUIRED)

## #################################################################
## Dependencies - OpenCV
## #################################################################

hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)

hunter_add_package(eos)
find_package(eos CONFIG REQUIRED)

hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)

hunter_add_package(Eigen)

string(COMPARE EQUAL "${Eigen_LICENSES}" "" is_empty)
if(is_empty)
  message(FATAL_ERROR "Licenses not found")
endif()

message("Eigen licenses:")
foreach(x ${Eigen_LICENSES})
  message("* ${x}")
  if(NOT EXISTS "${Eigen_LICENSES}")
    message(FATAL_ERROR "File not found")
  endif()
endforeach()

find_package(Eigen3 CONFIG REQUIRED)

# Test double library creation
find_package(Eigen3 CONFIG REQUIRED)

hunter_add_package(cxxopts)
find_package(cxxopts CONFIG REQUIRED)

hunter_add_package(OpenCV)
find_package(OpenCV REQUIRED)

hunter_add_package(dlib)
find_package(dlib CONFIG REQUIRED)

## #################################################################
## Dependencies - cereal (lightweight boost serialization alt)
## #################################################################

hunter_add_package(cereal)
find_package(cereal CONFIG REQUIRED)

## #################################################################
## Dependencies - libyuv
## #################################################################

hunter_add_package(libyuv)
find_package(libyuv CONFIG REQUIRED)

# add_subdirectory(external/nanomsg)
add_subdirectory(external/nng)

include_directories(
	tstunity
)

set ( TSTUNITY_CORE 
	tstunity/math_helper.c
	tstunity/sort_helper.c 
)

if (APPLE)
    if (IOS)
        set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)")
        add_library(tstunity STATIC
           ${TSTUNITY_CORE}
        )
    else ()
        set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD_32_64_BIT)")
        add_library(tstunity MODULE
            ${TSTUNITY_CORE}
        )
        set_target_properties ( tstunity PROPERTIES BUNDLE TRUE )
    endif ()
else ( )
	add_definitions("-DLIBTST_BUILD")
    add_library(tstunity SHARED
        ${TSTUNITY_CORE}
    )
endif ( )
	
if ( WIN32 AND NOT CYGWIN )
    target_link_libraries(tstunity
		ws2_32
		${THIRDPART_LIB} 
    )
else ( )
    target_link_libraries(tstunity
		m
		${THIRDPART_LIB} 
	)
endif ( )

# https://github.com/mixpanel/mixpanel-unity/blob/master/native/build/CMakeLists.txt#L186-L192

#install(TARGETS tstunity
#        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/../../deployments/native/lib"
#        LIBRARY DESTINATION "${PROJECT_SOURCE_DIR}/../../deployments/native/lib"
#        ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/../../deployments/native/lib")


