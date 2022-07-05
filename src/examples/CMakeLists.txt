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
set(ROBOMETRY_BUFFER_EXAMPLE_SRC robometry_buffer_example.cpp)
set(ROBOMETRY_BUFFER_MANAGER_CONF_FILE robometry_buffer_manager_conf_file.cpp)
set(ROBOMETRY_BUFFER_MANAGER_EXAMPLE_SRC robometry_buffer_manager_example.cpp)
set(ROBOMETRY_BUFFER_PERIODIC_SAVE_SRC robometry_buffer_periodic_save.cpp)
set(CB_TO_MATIO_EXAMPLE_SRC CB_to_matfile_example.cpp)


add_executable(circular_buffer_example ${CIRCULAR_BUFFER_EXAMPLE_SRC})
add_executable(circular_buffer_record_example ${CIRCULAR_BUFFER_RECORD_EXAMPLE_SRC})
add_executable(matio_vector_example ${MATIO_VECTOR_EXAMPLE_SRC})
add_executable(matio_matrix_example ${MATIO_MATRIX_EXAMPLE_SRC})
add_executable(matio_timeseries_example ${MATIO_TIMESERIES_EXAMPLE_SRC})
add_executable(robometry_buffer_example ${ROBOMETRY_BUFFER_EXAMPLE_SRC})
add_executable(robometry_buffer_manager_conf_file_example ${ROBOMETRY_BUFFER_MANAGER_CONF_FILE})
add_executable(robometry_buffer_manager_example ${ROBOMETRY_BUFFER_MANAGER_EXAMPLE_SRC})
add_executable(robometry_buffer_periodic_save ${ROBOMETRY_BUFFER_PERIODIC_SAVE_SRC})
add_executable(CB_to_matfile_example ${CB_TO_MATIO_EXAMPLE_SRC})

target_compile_features(circular_buffer_example PUBLIC cxx_std_17)
target_compile_features(circular_buffer_record_example PUBLIC cxx_std_17)
target_compile_features(robometry_buffer_example PUBLIC cxx_std_17)
target_compile_features(robometry_buffer_manager_conf_file_example PUBLIC cxx_std_17)
target_compile_features(robometry_buffer_manager_example PUBLIC cxx_std_17)
target_compile_features(robometry_buffer_periodic_save PUBLIC cxx_std_17)


target_link_libraries(circular_buffer_example Boost::boost)
target_link_libraries(circular_buffer_record_example Boost::boost
                                                     robometry::robometry)
target_link_libraries(robometry_buffer_example robometry::robometry)
target_link_libraries(robometry_buffer_manager_conf_file_example robometry::robometry)
target_link_libraries(robometry_buffer_manager_example robometry::robometry)
target_link_libraries(robometry_buffer_periodic_save robometry::robometry)
target_link_libraries(matio_vector_example PRIVATE matioCpp::matioCpp)
target_link_libraries(matio_matrix_example PRIVATE matioCpp::matioCpp)
target_link_libraries(matio_timeseries_example PRIVATE matioCpp::matioCpp)
target_link_libraries(CB_to_matfile_example PRIVATE matioCpp::matioCpp
                                                Boost::boost
                                                robometry::robometry
                                                ${CMAKE_THREAD_LIBS_INIT})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/conf/test_json.json
        DESTINATION ${CMAKE_BINARY_DIR}/bin)
