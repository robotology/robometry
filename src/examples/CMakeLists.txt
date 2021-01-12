# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

set(CIRCULAR_BUFFER_EXAMPLE_SRC circular_buffer_example.cpp)
set(CIRCULAR_BUFFER_RECORD_EXAMPLE_SRC circular_buffer_record_example.cpp)
set(MATIO_VECTOR_EXAMPLE_SRC matio_vector_example.cpp)
set(MATIO_MATRIX_EXAMPLE_SRC matio_matrix_example.cpp)
set(MATIO_TIMESERIES_EXAMPLE_SRC matio_timeseries_example.cpp)
set(TELEMETRY_BUFFER_EXAMPLE_SRC telemetry_buffer_example.cpp)
set(TELEMETRY_BUFFER_MANAGER_EXAMPLE_SRC telemetry_buffer_manager_example.cpp)
set(CB_TO_MATIO_EXAMPLE_SRC CB_to_matfile_example.cpp)


add_executable(circular_buffer_example ${CIRCULAR_BUFFER_EXAMPLE_SRC})
add_executable(circular_buffer_record_example ${CIRCULAR_BUFFER_RECORD_EXAMPLE_SRC})
add_executable(matio_vector_example ${MATIO_VECTOR_EXAMPLE_SRC})
add_executable(matio_matrix_example ${MATIO_MATRIX_EXAMPLE_SRC})
add_executable(matio_timeseries_example ${MATIO_TIMESERIES_EXAMPLE_SRC})
#add_executable(telemetry_buffer_example ${TELEMETRY_BUFFER_EXAMPLE_SRC})
add_executable(telemetry_buffer_manager_example ${TELEMETRY_BUFFER_MANAGER_EXAMPLE_SRC})
#add_executable(CB_to_matfile_example ${CB_TO_MATIO_EXAMPLE_SRC})

target_compile_features(circular_buffer_example PUBLIC cxx_std_14)
target_compile_features(circular_buffer_record_example PUBLIC cxx_std_14)
#target_compile_features(telemetry_buffer_example PUBLIC cxx_std_14)
target_compile_features(telemetry_buffer_manager_example PUBLIC cxx_std_14)


target_link_libraries(circular_buffer_example Boost::boost)
target_link_libraries(circular_buffer_record_example Boost::boost
                                                     YARP::YARP_conf
                                                     YARP::YARP_os
                                                     YARP::YARP_init
                                                     YARP::YARP_telemetry)
#target_link_libraries(telemetry_buffer_example YARP::YARP_conf
#                                               YARP::YARP_os
#                                               YARP::YARP_init
#                                               YARP::YARP_telemetry)
target_link_libraries(telemetry_buffer_manager_example YARP::YARP_conf
                                                       YARP::YARP_os
                                                       YARP::YARP_init
                                                       YARP::YARP_telemetry)
target_link_libraries(matio_vector_example PRIVATE matioCpp::matioCpp)
target_link_libraries(matio_matrix_example PRIVATE matioCpp::matioCpp)
target_link_libraries(matio_timeseries_example PRIVATE matioCpp::matioCpp)
#target_link_libraries(CB_to_matfile_example PRIVATE matioCpp::matioCpp 
#                                                Boost::boost 
#                                                YARP::YARP_telemetry
#                                                YARP::YARP_conf
#                                                YARP::YARP_os
#                                                YARP::YARP_init ${CMAKE_THREAD_LIBS_INIT})
