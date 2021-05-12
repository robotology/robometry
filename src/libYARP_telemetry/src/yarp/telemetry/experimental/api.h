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

#include <yarp/conf/api.h>
#ifndef YARP_telemetry_API
#  ifdef YARP_telemetry_EXPORTS
#    define YARP_telemetry_API YARP_EXPORT
#    define YARP_telemetry_EXTERN YARP_EXPORT_EXTERN
#  else
#    define YARP_telemetry_API YARP_IMPORT
#    define YARP_telemetry_EXTERN YARP_IMPORT_EXTERN
#  endif
#endif

#endif // YARP_TELEMETRY_API_H
