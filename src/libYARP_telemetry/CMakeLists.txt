# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

project(YARP_telemetry)

add_subdirectory(src)

include(InstallBasicPackageFiles)

install_basic_package_files(YARP_telemetry
                                COMPATIBILITY AnyNewerVersion
                                VERSION ${yarp-telemetry_VERSION}
                                DEPENDENCIES ${YARP_telemetry_PUBLIC_DEPS}
                                PRIVATE_DEPENDENCIES ${YARP_telemetry_PRIVATE_DEPS}
                                NAMESPACE YARP::
                                EXPORT_DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
