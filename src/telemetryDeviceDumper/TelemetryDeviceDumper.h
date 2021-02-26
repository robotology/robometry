/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef YARP_TELEMETRY_TELEMETRYDEVICEDUMPER_H
#define YARP_TELEMETRY_TELEMETRYDEVICEDUMPER_H

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/IWrapper.h>
#include <yarp/dev/IMultipleWrapper.h>
#include <yarp/os/PeriodicThread.h>
#include <yarp/telemetry/BufferManager.h>

namespace yarp::telemetry {
/**
 * @brief FILL DOCUMENTATION
 *
 */
class TelemetryDeviceDumper : public yarp::dev::DeviceDriver,
                              public yarp::dev::IWrapper,
                              public yarp::dev::IMultipleWrapper,
                              public yarp::os::PeriodicThread
{
public:
    TelemetryDeviceDumper();
    ~TelemetryDeviceDumper() override;

    //DeviceDriver
    bool close() override;
    /**
        * Configure with a set of options.
        * @param config The options to use
        * @return true iff the object could be configured.
        */
    bool open(yarp::os::Searchable& config) override;

    // IMultipleWrapper interface
    bool        attachAll(const yarp::dev::PolyDriverList& device2attach) override;

    bool        detachAll() override;

    // IWrapper interface
    bool        attach(yarp::dev::PolyDriver* poly) override;

    bool        detach() override;

    // PeriodicThread
    bool threadInit() override;

    void threadRelease() override;

    void run() override;

};
}

#endif // YARP_TELEMETRY_TELEMETRYDEVICEDUMPER_H
