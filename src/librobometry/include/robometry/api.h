/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * Copyright (C) 2006-2010 RobotCub Consortium
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef ROBOMETRY_API_H
#define ROBOMETRY_API_H

#ifdef __GNUC__
#  ifndef __clang__
#    define ROBOMETRY_COMPILER_IS_GNU
#  else
#    ifdef __APPLE__
#      define ROBOMETRY_COMPILER_IS_AppleClang
#    else
#      define ROBOMETRY_COMPILER_IS_Clang
#    endif
#  endif
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
#  define ROBOMETRY_EXPORT __declspec(dllexport)
#  define ROBOMETRY_IMPORT __declspec(dllimport)
#elif (defined(ROBOMETRY_COMPILER_IS_GNU) && (__GNUC__ >= 4)) || defined(ROBOMETRY_COMPILER_IS_Clang) || defined(ROBOMETRY_COMPILER_IS_AppleClang)
#  define ROBOMETRY_EXPORT __attribute__ ((visibility ("default")))
#  define ROBOMETRY_IMPORT __attribute__ ((visibility ("default")))
#else
#  define ROBOMETRY_EXPORT
#  define ROBOMETRY_IMPORT
#endif

#ifndef ROBOMETRY_API
#  ifdef ROBOMETRY_EXPORTS
#    define ROBOMETRY_API ROBOMETRY_EXPORT
#  else
#    define ROBOMETRY_API ROBOMETRY_IMPORT
#  endif
#endif

#  define ROBOMETRY_UNUSED(x) (void)x;

#endif // ROBOMETRY_API_H
