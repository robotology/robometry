# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

set(CIRCULAR_BUFFER_EXAMPLE_SRC circular-buffer-example.cpp)
add_executable(circular-buffer-example ${CIRCULAR_BUFFER_EXAMPLE_SRC})
target_compile_features(circular-buffer-example PUBLIC cxx_std_14)
target_link_libraries(circular-buffer-example Boost::boost)
