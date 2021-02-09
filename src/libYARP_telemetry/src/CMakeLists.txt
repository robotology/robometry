# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

add_library(YARP_telemetry)
add_library(YARP::YARP_telemetry ALIAS YARP_telemetry)

set(YARP_telemetry_HDRS
  yarp/telemetry/api.h
  yarp/telemetry/Test.h
  yarp/telemetry/Record.h
  yarp/telemetry/Buffer.h
  yarp/telemetry/BufferConfig.h
  yarp/telemetry/BufferManager.h
)
set(YARP_telemetry_SRCS
  yarp/telemetry/Test.cpp
  yarp/telemetry/BufferConfig.cpp
)
set(YARP_telemetry_IMPL_HDRS )
set(YARP_telemetry_IMPL_SRCS )

source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  PREFIX "Source Files"
  FILES
    ${YARP_telemetry_SRCS}
    ${YARP_telemetry_IMPL_SRCS}
)
source_group(
  TREE "${CMAKE_CURRENT_SOURCE_DIR}"
  PREFIX "Header Files"
  FILES
    ${YARP_telemetry_HDRS}
    ${YARP_telemetry_IMPL_HDRS}
)

target_sources(YARP_telemetry
  PRIVATE
    ${YARP_telemetry_SRCS}
    ${YARP_telemetry_HDRS}
    ${YARP_telemetry_IMPL_SRCS}
    ${YARP_telemetry_IMPL_HDRS}
)


target_include_directories(YARP_telemetry
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(YARP_telemetry PUBLIC cxx_std_17)

target_link_libraries(YARP_telemetry PUBLIC  Boost::boost
                                             matioCpp::matioCpp
                                             ${CMAKE_THREAD_LIBS_INIT}
                                     PRIVATE nlohmann_json::nlohmann_json)
list(APPEND YARP_telemetry_PUBLIC_DEPS Boost
                                       matioCpp)
list(APPEND YARP_telemetry_PRIVATE_DEPS nlohmann_json)

set_property(TARGET YARP_telemetry PROPERTY PUBLIC_HEADER ${YARP_telemetry_HDRS})
set_property(TARGET YARP_telemetry PROPERTY PRIVATE_HEADER ${YARP_telemetry_IMPL_HDRS})
set_property(TARGET YARP_telemetry PROPERTY VERSION ${YARP_VERSION_SHORT})
set_property(TARGET YARP_telemetry PROPERTY SOVERSION 0)
set_property(TARGET YARP_telemetry PROPERTY FOLDER "Libraries")

install(
  TARGETS YARP_telemetry
  EXPORT YARP_telemetry
  RUNTIME
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT YARP_telemetry
  LIBRARY
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    COMPONENT YARP_telemetry
    NAMELINK_COMPONENT YARP_telemetry-dev
  ARCHIVE
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    COMPONENT YARP_telemetry-dev
  PUBLIC_HEADER
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/yarp/telemetry"
    COMPONENT YARP_telemetry-dev
  PRIVATE_HEADER
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/yarp/telemetry/impl"
    COMPONENT YARP_telemetry-priv-dev
)

set(YARP_telemetry_PUBLIC_DEPS ${YARP_telemetry_PUBLIC_DEPS} PARENT_SCOPE)
set(YARP_telemetry_PRIVATE_DEPS ${YARP_telemetry_PRIVATE_DEPS} PARENT_SCOPE)
