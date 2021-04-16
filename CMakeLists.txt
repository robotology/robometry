# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

cmake_minimum_required(VERSION 3.12)
project(yarp-telemetry)

include(GNUInstallDirs)
include(FeatureSummary)

find_package(YCM 0.12 REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Control where libraries and executables are placed during the build.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

# Under MSVC, we set CMAKE_DEBUG_POSTFIX to "d" to add a trailing "d" to library
# built in debug mode.
if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

# Build position independent code.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Disable C and C++ compiler extensions.
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.19)
  cmake_policy(SET CMP0111 NEW)
endif()


# Enable RPATH support for installed binaries and libraries
include(AddInstallRPATHSupport)
add_install_rpath_support(LIB_DIRS "${CMAKE_INSTALL_FULL_LIBDIR}"       # Libraries
                          BIN_DIRS "${CMAKE_INSTALL_FULL_BINDIR}"       # Binaries
                                   "${CMAKE_INSTALL_FULL_LIBDIR}/yarp"  # Plugins
                          INSTALL_NAME_DIR "${CMAKE_INSTALL_FULL_LIBDIR}"
                          USE_LINK_PATH)

# Encourage user to specify a build type (e.g. Release, Debug, etc.), otherwise set it to Release.
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif()
endif()



set(YARP_FORCE_DYNAMIC_PLUGINS TRUE CACHE INTERNAL "${PROJECT_NAME} is always built with dynamic plugins")
find_package(YARP 3.4 COMPONENTS conf dev os REQUIRED)
# Use the same build options as YARP
set(BUILD_SHARED_LIBS ${YARP_IS_SHARED_LIBRARY})
set(CMAKE_C_FLAGS ${YARP_C_FLAGS})
set(CMAKE_CXX_FLAGS ${YARP_CXX_FLAGS})


#### INSERT find_package(MATIO) and other libraries here
find_package(matioCpp REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

option(YARP_TELEMETRY_USES_SYSTEM_nlohmann_json OFF)
# 3.9.2 is unreleased, this option is not working until that version is not released
if(YARP_TELEMETRY_USES_SYSTEM_nlohmann_json)
  find_package(nlohmann_json 3.9.2 REQUIRED)
else()
  include(FetchContent)

  FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG develop) # awaiting 3.9.2 release for https://github.com/nlohmann/json/issues/2513

  FetchContent_GetProperties(json)
  if(NOT json_POPULATED)
    message(STATUS "Fetching nlohmann_json...")
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
endif()


feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES)

add_subdirectory(src)

option(BUILD_TESTING "Create tests using CMake" OFF)
option(YARP_TELEMETRY_USES_SYSTEM_Catch2 OFF)

##############################
########### Test #############
##############################

# Just copied from yarp
cmake_dependent_option(YARP_VALGRIND_TESTS
                       "Run YARP tests under Valgrind" OFF
                       "BUILD_TESTING" OFF)
cmake_dependent_option(ENABLE_BENCHMARKING
                       "Enable the benchmarking in the unit tests" OFF
                       "BUILD_TESTING" OFF)
mark_as_advanced(YARP_VALGRIND_TESTS)

if(YARP_VALGRIND_TESTS)
  find_program(VALGRIND_EXECUTABLE NAMES valgrind)
  mark_as_advanced(VALGRIND_EXECUTABLE)

  if(VALGRIND_EXECUTABLE)
    set(VALGRIND_OPTIONS "--tool=memcheck --leak-check=full"
      CACHE STRING "Valgrind options (--error-exitcode=1 will be appended)")
    mark_as_advanced(VALGRIND_OPTIONS)
    separate_arguments(VALGRIND_OPTIONS UNIX_COMMAND "${VALGRIND_OPTIONS}")
    set(VALGRIND_COMMAND "${VALGRIND_EXECUTABLE}" ${VALGRIND_OPTIONS} --error-exitcode=1 --fullpath-after=${CMAKE_SOURCE_DIR}/)
  else()
    message(SEND_ERROR "Valgrind executable not found")
  endif()
endif()

unset(YARP_TEST_LAUNCHER)
set(YARP_TEST_TIMEOUT_DEFAULT_VALGRIND 300)
if(DEFINED VALGRIND_COMMAND)
  set(YARP_TEST_LAUNCHER ${VALGRIND_COMMAND})
  # The default timeout is not enough when running under valgrind
  if(YARP_TEST_TIMEOUT EQUAL YARP_TEST_TIMEOUT_DEFAULT)
    set_property(CACHE YARP_TEST_TIMEOUT PROPERTY VALUE ${YARP_TEST_TIMEOUT_DEFAULT_VALGRIND})
  endif()
else()
  if(YARP_TEST_TIMEOUT EQUAL YARP_TEST_TIMEOUT_DEFAULT_VALGRIND)
    set_property(CACHE YARP_TEST_TIMEOUT PROPERTY VALUE ${YARP_TEST_TIMEOUT_DEFAULT})
  endif()
endif()


if(BUILD_TESTING)

  if(YARP_TELEMETRY_USES_SYSTEM_Catch2)
    find_package(Catch2 2.13.1 REQUIRED)
  else()
    include(FetchContent)

    message(STATUS "Fetching Catch2...")

    FetchContent_Declare(Catch2
                         GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                         GIT_TAG        v2.13.1)

    FetchContent_MakeAvailable(Catch2)
   endif()
    enable_testing()
    add_subdirectory(test)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS 1)

include(AddUninstallTarget)
