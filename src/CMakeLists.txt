# Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

add_subdirectory(librobometry)
if(YARP_os_FOUND AND YARP_dev_FOUND)
  add_subdirectory(telemetryDeviceDumper)
endif()
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
