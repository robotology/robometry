/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * Copyright (C) 2006-2010 RobotCub Consortium
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef YARP_TELEMETRY_API_H
#define YARP_TELEMETRY_API_H

#ifdef __GNUC__
#  ifndef __clang__
#    define ROBOT_TELEMETRY_COMPILER_IS_GNU
#  else
#    ifdef __APPLE__
#      define ROBOT_TELEMETRY_COMPILER_IS_AppleClang
#    else
#      define ROBOT_TELEMETRY_COMPILER_IS_Clang
#    endif
#  endif
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
#  define ROBOT_TELEMETRY_EXPORT __declspec(dllexport)
#  define ROBOT_TELEMETRY_IMPORT __declspec(dllimport)
#elif (defined(ROBOT_TELEMETRY_COMPILER_IS_GNU) && (__GNUC__ >= 4)) || defined(ROBOT_TELEMETRY_COMPILER_IS_Clang) || defined(ROBOT_TELEMETRY_COMPILER_IS_AppleClang)
#  define ROBOT_TELEMETRY_EXPORT __attribute__ ((visibility ("default")))
#  define ROBOT_TELEMETRY_IMPORT __attribute__ ((visibility ("default")))
#else
#  define ROBOT_TELEMETRY_EXPORT
#  define ROBOT_TELEMETRY_IMPORT
#endif

#ifndef YARP_telemetry_API
#  ifdef ROBOT_TELEMETRY_EXPORTS
#    define YARP_telemetry_API ROBOT_TELEMETRY_EXPORT
#  else
#    define YARP_telemetry_API ROBOT_TELEMETRY_IMPORT
#  endif
#endif

#  define ROBOT_TELEMETRY_UNUSED(x) (void)x;

#endif // YARP_TELEMETRY_API_H
