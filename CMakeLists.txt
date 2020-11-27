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

# Enable RPATH support for installed binaries and libraries
include(AddInstallRPATHSupport)
add_install_rpath_support(BIN_DIRS "${CMAKE_INSTALL_FULL_BINDIR}"
                          LIB_DIRS "${CMAKE_INSTALL_FULL_LIBDIR}"
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
find_package(YARP 3.4 COMPONENTS conf REQUIRED)
# Use the same build options as YARP
set(BUILD_SHARED_LIBS ${YARP_IS_SHARED_LIBRARY})
set(CMAKE_C_FLAGS ${YARP_C_FLAGS})
set(CMAKE_CXX_FLAGS ${YARP_CXX_FLAGS})


#### INSERT find_package(MATIO) and other libraries here
find_package(matioCpp REQUIRED)


feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES)

add_subdirectory(src)

option(BUILD_TESTING "Create tests using CMake" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS 1)
