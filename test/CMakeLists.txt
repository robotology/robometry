# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

add_executable(bufferManager_test BufferManagerTest.cpp)
target_link_libraries(bufferManager_test PRIVATE Catch2::Catch2
                                                 matioCpp::matioCpp 
                                                 Boost::boost 
                                                 YARP::YARP_telemetry
                                                 YARP::YARP_conf
                                                 YARP::YARP_os
                                                 YARP::YARP_init)

include(CTest)
include(Catch)
catch_discover_tests(bufferManager_test)