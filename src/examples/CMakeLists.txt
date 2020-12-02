# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

set(CIRCULAR_BUFFER_EXAMPLE_SRC circular-buffer-example.cpp)
set(MATIO_VECTOR_EXAMPLE_SRC matio_vector_example.cpp)
set(MATIO_MATRIX_EXAMPLE_SRC matio_matrix_example.cpp)
set(MATIO_TIMESERIES_EXAMPLE_SRC matio_timeseries_example.cpp)


add_executable(circular-buffer-example ${CIRCULAR_BUFFER_EXAMPLE_SRC})
add_executable(matio_vector_example ${MATIO_VECTOR_EXAMPLE_SRC})
add_executable(matio_matrix_example ${MATIO_MATRIX_EXAMPLE_SRC})
add_executable(matio_timeseries_example ${MATIO_TIMESERIES_EXAMPLE_SRC})

target_compile_features(circular-buffer-example PUBLIC cxx_std_14)
target_link_libraries(circular-buffer-example Boost::boost)
target_link_libraries(matio_vector_example PRIVATE matioCpp::matioCpp)
target_link_libraries(matio_matrix_example PRIVATE matioCpp::matioCpp)
target_link_libraries(matio_timeseries_example PRIVATE matioCpp::matioCpp)
