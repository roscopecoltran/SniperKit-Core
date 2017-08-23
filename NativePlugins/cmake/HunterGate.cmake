# Copyright (c) 2013-2017, Ruslan Baratov
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This is a gate file to Hunter package manager.
# Include this file using `include` command and add package you need, example:
#
#     cmake_minimum_required(VERSION 3.0)
#
#     include("cmake/HunterGate.cmake")
#     HunterGate(
#         URL "https://github.com/path/to/hunter/archive.tar.gz"
#         SHA1 "798501e983f14b28b10cda16afa4de69eee1da1d"
#     )
#
#     project(MyProject)
#
#     hunter_add_package(Foo)
#     hunter_add_package(Boo COMPONENTS Bar Baz)
#
# Projects:
#     * https://github.com/hunter-packages/gate/
#     * https://github.com/ruslo/hunter

option(HUNTER_ENABLED "Enable Hunter package manager support" ON)
if(HUNTER_ENABLED)
  if(CMAKE_VERSION VERSION_LESS "3.0")
    message(FATAL_ERROR "At least CMake version 3.0 required for hunter dependency management."
      " Update CMake or set HUNTER_ENABLED to OFF.")
  endif()
endif()

include(CMakeParseArguments) # cmake_parse_arguments

option(HUNTER_STATUS_PRINT "Print working status" ON)
option(HUNTER_STATUS_DEBUG "Print a lot info" OFF)

set(HUNTER_WIKI "https://github.com/ruslo/hunter/wiki")


set(queued_libraries )
set(queued_executables )
set(queued_packages )

# Results:
# ARGC="3"
# ARGN="core;bitwriter"
# ARGV="foo;core;bitwriter"
# ARGV0="foo"
# ARGV1="core"
# https://gist.github.com/roscopecoltran/9a6c58a6e3ce7e8e616d3f23000d2512

# https://github.com/dealii/dealii/tree/master/cmake/macros
MACRO(ADD_LIBRARY _library_name)
  STRING(TOUPPER ${_library_name} _library_name_uppercase)
  message(STATUS " *** APPEND *** LIBRARY_NAME=${_library_name}, _library_name_uppercase=${_library_name_uppercase}")
  IF( NOT DEFINED ${_library_name_uppercase}_FOUND AND
      NOT DEFINED ${_library_name_uppercase}_LIBRARIES )
    _add_library (${_library_name} ${ARGN})
    list(APPEND queued_libraries ${_library_name})
    message(STATUS " *** APPEND *** LIBRARY NAME : ${_library_name}")
    message(STATUS " *** APPEND *** ADD LIBRARY *** ARGS : ${ARGN}")
  ELSE()
    IF(NOT DEFINED ${_library_name_uppercase}_FOUND)
      SET(${_library_name_uppercase}_FOUND TRUE)
    ENDIF()
  ENDIF()
ENDMACRO()

MACRO(FIND_PACKAGE _package_name)
  STRING(TOUPPER ${_package_name} _package_name_uppercase)
  message(STATUS " *** APPEND *** PACKAGE_NAME=${_package_name}, _package_name_uppercase=${_package_name_uppercase}")
  IF( NOT DEFINED ${_package_name_uppercase}_FOUND AND
      NOT DEFINED ${_package_name_uppercase}_LIBRARIES )
    _FIND_PACKAGE (${_package_name} ${ARGN})
    message(STATUS " *** APPEND *** FIND_PACKAGE *** ARGS : ${ARGN}")
    # buildx_copy_dependency(${_library_name} tstunity ${export_dir})
  ELSE()
    IF(NOT DEFINED ${_package_name_uppercase}_FOUND)
      SET(${_package_name_uppercase}_FOUND TRUE)
    ENDIF()
  ENDIF()

foreach(queued_library ${queued_libraries})
  message(STATUS " #### - ${queued_library}")  
  string(REPLACE "::" ";" NAMESPACE_ROOT ${queued_library})    
  # hunter_add_package(foo)
  # find_package(foo CONFIG REQUIRED) # introduce foo::foo target
  add_custom_command(
      OUTPUT "$<TARGET_FILE:${queued_library}>"
      COMMAND
      "${CMAKE_COMMAND}" -E copy
      "$<TARGET_FILE:${queued_library}>"
      "../PluginsBuild/NativePlugins/iOS"
      # WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  )

endforeach()

ENDMACRO()

MACRO(ADD_EXECUTABLE _executable_name)
  STRING(TOUPPER ${_executable_name} _executable_name_uppercase)
  message(STATUS " *** PROCESSING *** EXECUTABLE_NAME=${_executable_name}, _executable_name_uppercase: ${_executable_name_uppercase}")
  IF( NOT DEFINED ${_executable_name_uppercase}_FOUND AND
      NOT DEFINED ${_executable_name_uppercase}_EXECUTABLES )
    _add_executable (${_executable_name} ${ARGN})
    message(STATUS " *** APPEND *** ADD EXECUTABLE *** ARGS : ${ARGN}")
  ELSE()
    IF(NOT DEFINED ${_executable_name_uppercase}_FOUND)
      SET(${_executable_name_uppercase}_FOUND TRUE)
    ENDIF()
  ENDIF()
ENDMACRO()

function(hunter_gate_status_print)
  foreach(print_message ${ARGV})
    if(HUNTER_STATUS_PRINT OR HUNTER_STATUS_DEBUG)
      message(STATUS "[hunter] ${print_message}")
    endif()
  endforeach()
endfunction()

function(hunter_gate_status_debug)
  foreach(print_message ${ARGV})
    if(HUNTER_STATUS_DEBUG)
      string(TIMESTAMP timestamp)
      message(STATUS "[hunter *** DEBUG *** ${timestamp}] ${print_message}")
    endif()
  endforeach()
endfunction()

function(hunter_gate_wiki wiki_page)
  message("------------------------------ WIKI -------------------------------")
  message("    ${HUNTER_WIKI}/${wiki_page}")
  message("-------------------------------------------------------------------")
  message("")
  message(FATAL_ERROR "")
endfunction()

function(hunter_gate_internal_error)
  message("")
  foreach(print_message ${ARGV})
    message("[hunter ** INTERNAL **] ${print_message}")
  endforeach()
  message("[hunter ** INTERNAL **] [Directory:${CMAKE_CURRENT_LIST_DIR}]")
  message("")
  hunter_gate_wiki("error.internal")
endfunction()

function(hunter_gate_fatal_error)
  cmake_parse_arguments(hunter "" "WIKI" "" "${ARGV}")
  string(COMPARE EQUAL "${hunter_WIKI}" "" have_no_wiki)
  if(have_no_wiki)
    hunter_gate_internal_error("Expected wiki")
  endif()
  message("")
  foreach(x ${hunter_UNPARSED_ARGUMENTS})
    message("[hunter ** FATAL ERROR **] ${x}")
  endforeach()
  message("[hunter ** FATAL ERROR **] [Directory:${CMAKE_CURRENT_LIST_DIR}]")
  message("")
  hunter_gate_wiki("${hunter_WIKI}")
endfunction()

function(hunter_gate_user_error)
  hunter_gate_fatal_error(${ARGV} WIKI "error.incorrect.input.data")
endfunction()

function(hunter_gate_self root version sha1 result)
  string(COMPARE EQUAL "${root}" "" is_bad)
  if(is_bad)
    hunter_gate_internal_error("root is empty")
  endif()

  string(COMPARE EQUAL "${version}" "" is_bad)
  if(is_bad)
    hunter_gate_internal_error("version is empty")
  endif()

  string(COMPARE EQUAL "${sha1}" "" is_bad)
  if(is_bad)
    hunter_gate_internal_error("sha1 is empty")
  endif()

  string(SUBSTRING "${sha1}" 0 7 archive_id)

  if(EXISTS "${root}/cmake/Hunter")
    set(hunter_self "${root}")
  else()
    set(
        hunter_self
        "${root}/_Base/Download/Hunter/${version}/${archive_id}/Unpacked"
    )
  endif()

  set("${result}" "${hunter_self}" PARENT_SCOPE)
endfunction()

# Set HUNTER_GATE_ROOT cmake variable to suitable value.
function(hunter_gate_detect_root)
  # Check CMake variable
  string(COMPARE NOTEQUAL "${HUNTER_ROOT}" "" not_empty)
  if(not_empty)
    set(HUNTER_GATE_ROOT "${HUNTER_ROOT}" PARENT_SCOPE)
    hunter_gate_status_debug("HUNTER_ROOT detected by cmake variable")
    return()
  endif()

  # Check environment variable
  string(COMPARE NOTEQUAL "$ENV{HUNTER_ROOT}" "" not_empty)
  if(not_empty)
    set(HUNTER_GATE_ROOT "$ENV{HUNTER_ROOT}" PARENT_SCOPE)
    hunter_gate_status_debug("HUNTER_ROOT detected by environment variable")
    return()
  endif()

  # Check HOME environment variable
  string(COMPARE NOTEQUAL "$ENV{HOME}" "" result)
  if(result)
    set(HUNTER_GATE_ROOT "$ENV{HOME}/.hunter" PARENT_SCOPE)
    hunter_gate_status_debug("HUNTER_ROOT set using HOME environment variable")
    return()
  endif()

  # Check SYSTEMDRIVE and USERPROFILE environment variable (windows only)
  if(WIN32)
    string(COMPARE NOTEQUAL "$ENV{SYSTEMDRIVE}" "" result)
    if(result)
      set(HUNTER_GATE_ROOT "$ENV{SYSTEMDRIVE}/.hunter" PARENT_SCOPE)
      hunter_gate_status_debug(
          "HUNTER_ROOT set using SYSTEMDRIVE environment variable"
      )
      return()
    endif()

    string(COMPARE NOTEQUAL "$ENV{USERPROFILE}" "" result)
    if(result)
      set(HUNTER_GATE_ROOT "$ENV{USERPROFILE}/.hunter" PARENT_SCOPE)
      hunter_gate_status_debug(
          "HUNTER_ROOT set using USERPROFILE environment variable"
      )
      return()
    endif()
  endif()

  hunter_gate_fatal_error(
      "Can't detect HUNTER_ROOT"
      WIKI "error.detect.hunter.root"
  )
endfunction()

macro(hunter_gate_lock dir)
  if(NOT HUNTER_SKIP_LOCK)
    if("${CMAKE_VERSION}" VERSION_LESS "3.2")
      hunter_gate_fatal_error(
          "Can't lock, upgrade to CMake 3.2 or use HUNTER_SKIP_LOCK"
          WIKI "error.can.not.lock"
      )
    endif()
    hunter_gate_status_debug("Locking directory: ${dir}")
    file(LOCK "${dir}" DIRECTORY GUARD FUNCTION)
    hunter_gate_status_debug("Lock done")
  endif()
endmacro()

function(hunter_gate_download dir)
  string(
      COMPARE
      NOTEQUAL
      "$ENV{HUNTER_DISABLE_AUTOINSTALL}"
      ""
      disable_autoinstall
  )
  if(disable_autoinstall AND NOT HUNTER_RUN_INSTALL)
    hunter_gate_fatal_error(
        "Hunter not found in '${dir}'"
        "Set HUNTER_RUN_INSTALL=ON to auto-install it from '${HUNTER_GATE_URL}'"
        "Settings:"
        "  HUNTER_ROOT: ${HUNTER_GATE_ROOT}"
        "  HUNTER_SHA1: ${HUNTER_GATE_SHA1}"
        WIKI "error.run.install"
    )
  endif()
  string(COMPARE EQUAL "${dir}" "" is_bad)
  if(is_bad)
    hunter_gate_internal_error("Empty 'dir' argument")
  endif()

  string(COMPARE EQUAL "${HUNTER_GATE_SHA1}" "" is_bad)
  if(is_bad)
    hunter_gate_internal_error("HUNTER_GATE_SHA1 empty")
  endif()

  string(COMPARE EQUAL "${HUNTER_GATE_URL}" "" is_bad)
  if(is_bad)
    hunter_gate_internal_error("HUNTER_GATE_URL empty")
  endif()

  set(done_location "${dir}/DONE")
  set(sha1_location "${dir}/SHA1")

  set(build_dir "${dir}/Build")
  set(cmakelists "${dir}/CMakeLists.txt")

  hunter_gate_lock("${dir}")
  if(EXISTS "${done_location}")
    # while waiting for lock other instance can do all the job
    hunter_gate_status_debug("File '${done_location}' found, skip install")
    return()
  endif()

  file(REMOVE_RECURSE "${build_dir}")
  file(REMOVE_RECURSE "${cmakelists}")

  file(MAKE_DIRECTORY "${build_dir}") # check directory permissions

  # Disabling languages speeds up a little bit, reduces noise in the output
  # and avoids path too long windows error
  file(
      WRITE
      "${cmakelists}"
      "cmake_minimum_required(VERSION 3.0)\n"
      "project(HunterDownload LANGUAGES NONE)\n"
      "include(ExternalProject)\n"
      "ExternalProject_Add(\n"
      "    Hunter\n"
      "    URL\n"
      "    \"${HUNTER_GATE_URL}\"\n"
      "    URL_HASH\n"
      "    SHA1=${HUNTER_GATE_SHA1}\n"
      "    DOWNLOAD_DIR\n"
      "    \"${dir}\"\n"
      "    SOURCE_DIR\n"
      "    \"${dir}/Unpacked\"\n"
      "    CONFIGURE_COMMAND\n"
      "    \"\"\n"
      "    BUILD_COMMAND\n"
      "    \"\"\n"
      "    INSTALL_COMMAND\n"
      "    \"\"\n"
      ")\n"
  )

  if(HUNTER_STATUS_DEBUG)
    set(logging_params "")
  else()
    set(logging_params OUTPUT_QUIET)
  endif()

  hunter_gate_status_debug("Run generate")

  # Need to add toolchain file too.
  # Otherwise on Visual Studio + MDD this will fail with error:
  # "Could not find an appropriate version of the Windows 10 SDK installed on this machine"
  if(EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    get_filename_component(absolute_CMAKE_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)
    set(toolchain_arg "-DCMAKE_TOOLCHAIN_FILE=${absolute_CMAKE_TOOLCHAIN_FILE}")
  else()
    # 'toolchain_arg' can't be empty
    set(toolchain_arg "-DCMAKE_TOOLCHAIN_FILE=")
  endif()

  string(COMPARE EQUAL "${CMAKE_MAKE_PROGRAM}" "" no_make)
  if(no_make)
    set(make_arg "")
  else()
    # Test case: remove Ninja from PATH but set it via CMAKE_MAKE_PROGRAM
    set(make_arg "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
  endif()

  execute_process(
      COMMAND
      "${CMAKE_COMMAND}"
      "-H${dir}"
      "-B${build_dir}"
      "-G${CMAKE_GENERATOR}"
      "${toolchain_arg}"
      ${make_arg}
      WORKING_DIRECTORY "${dir}"
      RESULT_VARIABLE download_result
      ${logging_params}
  )

  if(NOT download_result EQUAL 0)
    hunter_gate_internal_error("Configure project failed")
  endif()

  hunter_gate_status_print(
      "Initializing Hunter workspace (${HUNTER_GATE_SHA1})"
      "  ${HUNTER_GATE_URL}"
      "  -> ${dir}"
  )
  execute_process(
      COMMAND "${CMAKE_COMMAND}" --build "${build_dir}"
      WORKING_DIRECTORY "${dir}"
      RESULT_VARIABLE download_result
      ${logging_params}
  )

  if(NOT download_result EQUAL 0)
    hunter_gate_internal_error("Build project failed")
  endif()

  file(REMOVE_RECURSE "${build_dir}")
  file(REMOVE_RECURSE "${cmakelists}")

  file(WRITE "${sha1_location}" "${HUNTER_GATE_SHA1}")
  file(WRITE "${done_location}" "DONE")

  hunter_gate_status_debug("Finished")
endfunction()

# Must be a macro so master file 'cmake/Hunter' can
# apply all variables easily just by 'include' command
# (otherwise PARENT_SCOPE magic needed)
macro(HunterGate)
  if(HUNTER_GATE_DONE)
    # variable HUNTER_GATE_DONE set explicitly for external project
    # (see `hunter_download`)
    set_property(GLOBAL PROPERTY HUNTER_GATE_DONE YES)
  endif()

  # First HunterGate command will init Hunter, others will be ignored
  get_property(_hunter_gate_done GLOBAL PROPERTY HUNTER_GATE_DONE SET)

  if(NOT HUNTER_ENABLED)
    # Empty function to avoid error "unknown function"
    function(hunter_add_package)
    endfunction()

    set(
        _hunter_gate_disabled_mode_dir
        "${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/disabled-mode"
    )
    if(EXISTS "${_hunter_gate_disabled_mode_dir}")
      hunter_gate_status_debug(
          "Adding \"disabled-mode\" modules: ${_hunter_gate_disabled_mode_dir}"
      )
      list(APPEND CMAKE_PREFIX_PATH "${_hunter_gate_disabled_mode_dir}")
    endif()
  elseif(_hunter_gate_done)
    hunter_gate_status_debug("Secondary HunterGate (use old settings)")
    hunter_gate_self(
        "${HUNTER_CACHED_ROOT}"
        "${HUNTER_VERSION}"
        "${HUNTER_SHA1}"
        _hunter_self
    )
    include("${_hunter_self}/cmake/Hunter")
  else()
    set(HUNTER_GATE_LOCATION "${CMAKE_CURRENT_LIST_DIR}")

    string(COMPARE NOTEQUAL "${PROJECT_NAME}" "" _have_project_name)
    if(_have_project_name)
      hunter_gate_fatal_error(
          "Please set HunterGate *before* 'project' command. "
          "Detected project: ${PROJECT_NAME}"
          WIKI "error.huntergate.before.project"
      )
    endif()

    cmake_parse_arguments(
        HUNTER_GATE "LOCAL" "URL;SHA1;GLOBAL;FILEPATH" "" ${ARGV}
    )

    string(COMPARE EQUAL "${HUNTER_GATE_SHA1}" "" _empty_sha1)
    string(COMPARE EQUAL "${HUNTER_GATE_URL}" "" _empty_url)
    string(
        COMPARE
        NOTEQUAL
        "${HUNTER_GATE_UNPARSED_ARGUMENTS}"
        ""
        _have_unparsed
    )
    string(COMPARE NOTEQUAL "${HUNTER_GATE_GLOBAL}" "" _have_global)
    string(COMPARE NOTEQUAL "${HUNTER_GATE_FILEPATH}" "" _have_filepath)

    if(_have_unparsed)
      hunter_gate_user_error(
          "HunterGate unparsed arguments: ${HUNTER_GATE_UNPARSED_ARGUMENTS}"
      )
    endif()
    if(_empty_sha1)
      hunter_gate_user_error("SHA1 suboption of HunterGate is mandatory")
    endif()
    if(_empty_url)
      hunter_gate_user_error("URL suboption of HunterGate is mandatory")
    endif()
    if(_have_global)
      if(HUNTER_GATE_LOCAL)
        hunter_gate_user_error("Unexpected LOCAL (already has GLOBAL)")
      endif()
      if(_have_filepath)
        hunter_gate_user_error("Unexpected FILEPATH (already has GLOBAL)")
      endif()
    endif()
    if(HUNTER_GATE_LOCAL)
      if(_have_global)
        hunter_gate_user_error("Unexpected GLOBAL (already has LOCAL)")
      endif()
      if(_have_filepath)
        hunter_gate_user_error("Unexpected FILEPATH (already has LOCAL)")
      endif()
    endif()
    if(_have_filepath)
      if(_have_global)
        hunter_gate_user_error("Unexpected GLOBAL (already has FILEPATH)")
      endif()
      if(HUNTER_GATE_LOCAL)
        hunter_gate_user_error("Unexpected LOCAL (already has FILEPATH)")
      endif()
    endif()

    hunter_gate_detect_root() # set HUNTER_GATE_ROOT

    # Beautify path, fix probable problems with windows path slashes
    get_filename_component(
        HUNTER_GATE_ROOT "${HUNTER_GATE_ROOT}" ABSOLUTE
    )
    hunter_gate_status_debug("HUNTER_ROOT: ${HUNTER_GATE_ROOT}")
    if(NOT HUNTER_ALLOW_SPACES_IN_PATH)
      string(FIND "${HUNTER_GATE_ROOT}" " " _contain_spaces)
      if(NOT _contain_spaces EQUAL -1)
        hunter_gate_fatal_error(
            "HUNTER_ROOT (${HUNTER_GATE_ROOT}) contains spaces."
            "Set HUNTER_ALLOW_SPACES_IN_PATH=ON to skip this error"
            "(Use at your own risk!)"
            WIKI "error.spaces.in.hunter.root"
        )
      endif()
    endif()

    string(
        REGEX
        MATCH
        "[0-9]+\\.[0-9]+\\.[0-9]+[-_a-z0-9]*"
        HUNTER_GATE_VERSION
        "${HUNTER_GATE_URL}"
    )
    string(COMPARE EQUAL "${HUNTER_GATE_VERSION}" "" _is_empty)
    if(_is_empty)
      set(HUNTER_GATE_VERSION "unknown")
    endif()

    hunter_gate_self(
        "${HUNTER_GATE_ROOT}"
        "${HUNTER_GATE_VERSION}"
        "${HUNTER_GATE_SHA1}"
        _hunter_self
    )

    set(_master_location "${_hunter_self}/cmake/Hunter")
    if(EXISTS "${HUNTER_GATE_ROOT}/cmake/Hunter")
      # Hunter downloaded manually (e.g. by 'git clone')
      set(_unused "xxxxxxxxxx")
      set(HUNTER_GATE_SHA1 "${_unused}")
      set(HUNTER_GATE_VERSION "${_unused}")
    else()
      get_filename_component(_archive_id_location "${_hunter_self}/.." ABSOLUTE)
      set(_done_location "${_archive_id_location}/DONE")
      set(_sha1_location "${_archive_id_location}/SHA1")

      # Check Hunter already downloaded by HunterGate
      if(NOT EXISTS "${_done_location}")
        hunter_gate_download("${_archive_id_location}")
      endif()

      if(NOT EXISTS "${_done_location}")
        hunter_gate_internal_error("hunter_gate_download failed")
      endif()

      if(NOT EXISTS "${_sha1_location}")
        hunter_gate_internal_error("${_sha1_location} not found")
      endif()
      file(READ "${_sha1_location}" _sha1_value)
      string(COMPARE EQUAL "${_sha1_value}" "${HUNTER_GATE_SHA1}" _is_equal)
      if(NOT _is_equal)
        hunter_gate_internal_error(
            "Short SHA1 collision:"
            "  ${_sha1_value} (from ${_sha1_location})"
            "  ${HUNTER_GATE_SHA1} (HunterGate)"
        )
      endif()
      if(NOT EXISTS "${_master_location}")
        hunter_gate_user_error(
            "Master file not found:"
            "  ${_master_location}"
            "try to update Hunter/HunterGate"
        )
      endif()
    endif()
    include("${_master_location}")
    set_property(GLOBAL PROPERTY HUNTER_GATE_DONE YES)
  endif()
endmacro()

function(buildx_print_target_property tgt prop)
  # v for value, d for defined, s for set
  get_property(v TARGET ${tgt} PROPERTY ${prop})
  get_property(d TARGET ${tgt} PROPERTY ${prop} DEFINED)
  get_property(s TARGET ${tgt} PROPERTY ${prop} SET)
  # only produce output for values that are set
  if(s OR d)
    message("tgt='${tgt}' prop='${prop}'")
    message("  value='${v}'")
    message("  defined='${d}'")
    message("  set='${s}'")
    message("")
  endif()
endfunction()
 
function(buildx_print_target_properties tgt)
  if(NOT TARGET ${tgt})
    message("There is no target named '${tgt}'")
    return()
  endif()

  set(props
    # ALIASED_TARGET
    EXPORT_NAME
    # IMPORTED_CONFIGURATIONS
    IMPORTED
    # MAP_IMPORTED_CONFIG_DEBUG
    # MAP_IMPORTED_CONFIG_RELEASE
    NAME
    # NO_SYSTEM_FROM_IMPORTED
    TYPE
  )

  message("======================== ${tgt} ========================")
  foreach(p ${props})
    buildx_print_target_property("${tgt}" "${p}")
  endforeach()
  message("")
endfunction()

macro(buildx_enable_debug)

  set(BUILDX_DEBUG_CATS ${ARGV})
  message("# buildx_debug enabled for {${BUILDX_DEBUG_CATS}}")

endmacro(buildx_enable_debug)

# make given variables available in the parent scope
macro(buildx_global)

  foreach(var ${ARGV})
    set(${var} ${${var}} PARENT_SCOPE)
  endforeach()

endmacro()

function(buildx_debug _msg)

  if(DEFINED BUILDX_DEBUG_CATS)
  
    if(DEFINED ARGV1)
      list(FIND BUILDX_DEBUG_CATS "all" CAT_FOUND)
      
      # check if cat should be printed
      if(CAT_FOUND EQUAL "-1")
        list(FIND BUILDX_DEBUG_CATS ${ARGV1} CAT_FOUND)
      endif()
    endif()
  
    if(NOT CAT_FOUND EQUAL "-1")
    
      message(STATUS "# ${_msg}")
    
    endif()
  
  endif()
  
endfunction()

function(copy_imported_targets _target)
  foreach(_dep ${ARGN})
    if(WIN32)
      add_custom_command(TARGET ${_target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${_dep}> $<TARGET_FILE_DIR:${_target}>
        COMMENT "Copying required DLL for dependency ${_dep}"
        VERBATIM)
    endif()
  endforeach()
endfunction()


function(install_imported_target _dep)
    install(FILES $<TARGET_FILE:${_dep}> ${ARGN})
endfunction()

function(buildx_copy_dependency _target _to _path)

  buildx_debug("Examining target '${_target}'" dep)
  if(NOT TARGET ${_target})
    message(FATAL_ERROR "'${_target}' is not a target yet! Forgott to import?")
  endif()

  get_property(type TARGET ${_target} PROPERTY TYPE)
  get_property(imported TARGET ${_target} PROPERTY IMPORTED)
  if(NOT ${type} STREQUAL "STATIC_LIBRARY" AND ${imported})
    buildx_debug("Copy dynamic libraries for target '${_target}' to '${_to}'" dep)

    add_custom_command( TARGET ${_to} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E
              copy_if_different $<TARGET_PROPERTY:${_target},IMPORTED_LOCATION_$<UPPER_CASE:$<CONFIG>>> $<TARGET_FILE_DIR:${_to}>)

    # ${_path}) #
    buildx_print_target_properties(${_target})

  endif()

  # recursive
  get_property(linkl TARGET ${_target} PROPERTY INTERFACE_LINK_LIBRARIES)
  foreach(li ${linkl})
    if(TARGET ${li})
      buildx_copy_dependency("${li}" "${_to}" "${_path}")
    endif()
  endforeach()

endfunction()

function(buildx_copy_target_dependencies _target _path)

  buildx_debug("Examining target '${_target}'" dep)
  get_property(linkl TARGET ${_target} PROPERTY LINK_LIBRARIES)
  foreach(li ${linkl})    
    buildx_copy_dependency("${li}" "${_target}" "${_path}")
  endforeach()
endfunction()

set(__add_file_copy_target YES)

define_property(TARGET
  PROPERTY
  FILE_COPY_TARGET
  BRIEF_DOCS
  "File Copy target"
  FULL_DOCS
  "Is this a target created by add_file_copy_target?")

function(add_file_copy_target _target _dest)
  if(NOT ARGN)
    message(WARNING
      "In add_file_copy_target call for target ${_target}, no source files were specified!")
    return()
  endif()

  set(ALLFILES)
  set(SOURCES)
  foreach(fn ${ARGN})
    # Produce an absolute path to the input file
    if(IS_ABSOLUTE "${fn}")
      get_filename_component(fullpath "${fn}" ABSOLUTE)
      get_filename_component(fn "${fn}" NAME)
    else()
      get_filename_component(fullpath
        "${CMAKE_CURRENT_SOURCE_DIR}/${fn}"
        ABSOLUTE)
    endif()

    # Clean up output file name
    get_filename_component(absout "${_dest}/${fn}" ABSOLUTE)

    add_custom_command(OUTPUT "${absout}"
      COMMAND
      ${CMAKE_COMMAND}
      ARGS -E make_directory "${_dest}"
      COMMAND
      ${CMAKE_COMMAND}
      ARGS -E copy "${fullpath}" "${_dest}"
      MAIN_DEPENDENCY "${fullpath}"
      VERBATIM
      COMMENT "Copying ${fn} to ${absout}")
    list(APPEND SOURCES "${fullpath}")
    list(APPEND ALLFILES "${absout}")
  endforeach()

  # Custom target depending on all the file copy commands
  add_custom_target(${_target}
    SOURCES ${SOURCES}
    DEPENDS ${ALLFILES})

  set_property(TARGET ${_target} PROPERTY FILE_COPY_TARGET YES)
endfunction()

function(install_file_copy_target _target)
  get_target_property(_isFCT ${_target} FILE_COPY_TARGET)
  if(NOT _isFCT)
    message(WARNING
      "install_file_copy_target called on a target not created with add_file_copy_target!")
    return()
  endif()

  # Get sources
  get_target_property(_srcs ${_target} SOURCES)

  # Remove the "fake" file forcing build
  list(REMOVE_AT _srcs 0)

  # Forward the call to install
  install(PROGRAMS ${_srcs} ${ARGN})
endfunction()
