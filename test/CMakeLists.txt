# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

if(ROBOMETRY_VALGRIND_TESTS)
  add_definitions(-DWITH_VALGRIND)
endif()

add_executable(bufferManager_test BufferManagerTest.cpp)
if(ROBOMETRY_BENCHMARKING)
  target_compile_definitions(bufferManager_test PRIVATE -DCATCH_CONFIG_ENABLE_BENCHMARKING)
endif()
target_link_libraries(bufferManager_test PRIVATE Catch2::Catch2
                                                 matioCpp::matioCpp
                                                 Boost::boost
                                                 robometry::robometry)

include(CTest)
include(Catch)
function(robot_telemetry_catch_discover_tests _target)
  # Workaround to force catch_discover_tests to run tests under valgrind
  set_property(TARGET ${_target} PROPERTY CROSSCOMPILING_EMULATOR "${ROBOMETRY_TEST_LAUNCHER}")
  catch_discover_tests(
    ${_target}
    EXTRA_ARGS "-s" "--use-colour" "yes"
    PROPERTIES
      TIMEOUT ${ROBOMETRY_TEST_TIMEOUT}
      SKIP_RETURN_CODE 254
    )
endfunction()


robot_telemetry_catch_discover_tests(bufferManager_test)
