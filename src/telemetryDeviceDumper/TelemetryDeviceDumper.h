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
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/PeriodicThread.h>
#include <yarp/telemetry/BufferManager.h>

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

namespace yarp::telemetry {


struct TelemetryDeviceDumperSettings {
    bool logJointVelocity{ false };
    bool logJointAcceleration{ false };
    bool useRadians{ false };
    std::string experimentName{"telemetryDeviceDumper"};
};
/**
 * @brief FILL DOCUMENTATION
 *
 */
class TelemetryDeviceDumper : public yarp::dev::DeviceDriver,
                              //public yarp::dev::IWrapper,
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
    //bool        attach(yarp::dev::PolyDriver* poly) override;

    //bool        detach() override;

    void run() override;

private:

    bool loadSettingsFromConfig(yarp::os::Searchable& config);
    bool attachAllControlBoards(const yarp::dev::PolyDriverList& p_list);
    bool openRemapperControlBoard(yarp::os::Searchable& config);
    void readSensors();
    void resizeBuffers(int size);
    bool configBufferManager(yarp::os::Searchable& config);
    /** Remapped controlboard containg the axes for which the joint torques are estimated */
    yarp::dev::PolyDriver remappedControlBoard;
    struct
    {
        yarp::dev::IEncoders* encs;
        yarp::dev::IMultipleWrapper* multwrap;
    } remappedControlBoardInterfaces;

    std::mutex deviceMutex;
    std::atomic<bool> correctlyConfigured{ false }, sensorsReadCorrectly{false};
    std::vector<double> jointPos, jointVel, jointAcc;
    std::vector<std::string> jointNames;
    TelemetryDeviceDumperSettings settings;
    yarp::telemetry::BufferManager<double> bufferManager;


};
}

#endif // YARP_TELEMETRY_TELEMETRYDEVICEDUMPER_H
