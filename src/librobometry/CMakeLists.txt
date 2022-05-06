# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

add_library(robometry)
add_library(robometry::robometry ALIAS robometry)

set(ROBOMETRY_HDRS include/robometry/Buffer.h
                   include/robometry/BufferConfig.h
                   include/robometry/BufferManager.h
                   include/robometry/Record.h
                   include/robometry/TreeNode.h
)
set(ROBOMETRY_SRCS src/BufferConfig.cpp
                   src/Buffer.cpp
)
set(ROBOMETRY_IMPL_HDRS )
set(ROBOMETRY_IMPL_SRCS )

source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  PREFIX "Source Files"
  FILES
    ${ROBOMETRY_SRCS}
    ${ROBOMETRY_IMPL_SRCS}
)
source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  PREFIX "Header Files"
  FILES
    ${ROBOMETRY_HDRS}
    ${ROBOMETRY_IMPL_HDRS}
)

target_sources(robometry
  PRIVATE
    ${ROBOMETRY_SRCS}
    ${ROBOMETRY_HDRS}
    ${ROBOMETRY_IMPL_SRCS}
    ${ROBOMETRY_IMPL_HDRS}
)


target_include_directories(robometry
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(robometry PUBLIC cxx_std_17)

target_link_libraries(robometry PUBLIC  Boost::boost
                                        matioCpp::matioCpp
                                        Threads::Threads
                                PRIVATE nlohmann_json::nlohmann_json)
# Support using filesystem on GCC < 9.1,
# see https://en.cppreference.com/w/cpp/filesystem#Notes
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1))
  target_link_libraries(robometry PUBLIC stdc++fs)
endif()
list(APPEND ROBOMETRY_PUBLIC_DEPS Boost
                                       matioCpp
                                       Threads)
list(APPEND ROBOMETRY_PRIVATE_DEPS nlohmann_json)

set_target_properties(robometry PROPERTIES DEFINE_SYMBOL ROBOMETRY_EXPORTS)

set_property(TARGET robometry PROPERTY PUBLIC_HEADER ${ROBOMETRY_HDRS})
set_property(TARGET robometry PROPERTY PRIVATE_HEADER ${ROBOMETRY_IMPL_HDRS})
set_property(TARGET robometry PROPERTY VERSION ${${CMAKE_PROJECT_NAME}_VERSION})
set_property(TARGET robometry PROPERTY SOVERSION 0)
set_property(TARGET robometry PROPERTY FOLDER "Libraries")

install(
  TARGETS robometry
  EXPORT robometry
  RUNTIME
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT robometry
  LIBRARY
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    COMPONENT robometry
    NAMELINK_COMPONENT robometry-dev
  ARCHIVE
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    COMPONENT robometry-dev
  PUBLIC_HEADER
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/robometry"
    COMPONENT robometry-dev
  PRIVATE_HEADER
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/yarp/telemetry/impl"
    COMPONENT robometry-priv-dev
)

set(ROBOMETRY_PUBLIC_DEPS ${ROBOMETRY_PUBLIC_DEPS} PARENT_SCOPE)
set(ROBOMETRY_PRIVATE_DEPS ${ROBOMETRY_PRIVATE_DEPS} PARENT_SCOPE)

include(InstallBasicPackageFiles)

install_basic_package_files(robometry
                                COMPATIBILITY AnyNewerVersion
                                VERSION ${robometry_VERSION}
                                DEPENDENCIES ${ROBOMETRY_PUBLIC_DEPS}
                                PRIVATE_DEPENDENCIES ${ROBOMETRY_PRIVATE_DEPS}
                                EXPORT_DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
