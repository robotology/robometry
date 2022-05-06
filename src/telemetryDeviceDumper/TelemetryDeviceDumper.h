/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef ROBOMETRY_TELEMETRYDEVICEDUMPER_H
#define ROBOMETRY_TELEMETRYDEVICEDUMPER_H

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/IWrapper.h>
#include <yarp/dev/IMultipleWrapper.h>
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/IMotorEncoders.h>
#include <yarp/dev/IPidControl.h>
#include <yarp/dev/IAmplifierControl.h>
#include <yarp/dev/IControlMode.h>
#include <yarp/dev/IInteractionMode.h>
#include <yarp/dev/ILocalization2D.h>
#include <yarp/dev/IMotor.h>
#include <yarp/dev/ITorqueControl.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/PeriodicThread.h>
#include <robometry/BufferManager.h>

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

namespace robometry {


struct TelemetryDeviceDumperSettings {

    YARP_DEPRECATED_MSG("logJointVelocity is deprecated, use logIEncoders instead.")
    bool logJointVelocity{ false };
    YARP_DEPRECATED_MSG("logJointAcceleration is deprecated, use logIEncoders instead.")
    bool logJointAcceleration{ false };

    bool logControlBoardQuantities{ false };
    bool logIEncoders{ true };
    bool logITorqueControl{ false };
    bool logIMotorEncoders{ false };
    bool logIControlMode{ false };
    bool logIInteractionMode{ false };
    bool logIPidControl{ false };
    bool logIAmplifierControl{ false };
    bool logILocalization2D{ false };
    bool useRadians{ false };
    bool saveBufferManagerConfiguration{ false };
    std::string localizationRemoteName{ "" };
};
/**
 * @brief FILL DOCUMENTATION
 *
 */
class TelemetryDeviceDumper : public yarp::dev::DeviceDriver,
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

    void run() override;

private:

    bool loadSettingsFromConfig(yarp::os::Searchable& config);
    bool attachAllControlBoards(const yarp::dev::PolyDriverList& p_list);
    bool openRemapperControlBoard(yarp::os::Searchable& config);
    void readSensors();
    void readOdometryData();
    void resizeBuffers(int size);
    bool configBufferManager(yarp::os::Searchable& config);
    /** Remapped controlboard containg the axes for which the joint torques are estimated */
    yarp::dev::PolyDriver remappedControlBoard, localization2DClient;
    struct
    {
        yarp::dev::IEncoders* encs{nullptr};
        yarp::dev::IMotorEncoders* imotenc{ nullptr };
        yarp::dev::IPidControl* pid{ nullptr };
        yarp::dev::IAmplifierControl* amp{nullptr};
        yarp::dev::IControlMode* cmod{ nullptr };
        yarp::dev::IInteractionMode* imod{ nullptr };
        yarp::dev::ITorqueControl* itrq{ nullptr };
        yarp::dev::IMultipleWrapper* multwrap{ nullptr };
    } remappedControlBoardInterfaces;

    yarp::dev::Nav2D::ILocalization2D* iloc{nullptr};

    std::mutex deviceMutex;
    std::atomic<bool> correctlyConfigured{ false }, sensorsReadCorrectly{false};
    std::vector<double> jointPos, jointVel, jointAcc, jointPosErr, jointPosRef,
                        jointTrqErr, jointTrqRef, jointPWM, jointCurr, jointTrq,
                        motorEnc, motorVel, motorAcc, controlModes, interactionModes,
                        odometryData;
    std::vector<std::string> jointNames;
    TelemetryDeviceDumperSettings settings;
    robometry::BufferConfig m_bufferConfig;
    robometry::BufferManager<double> bufferManager;


};
}

#endif // ROBOMETRY_TELEMETRYDEVICEDUMPER_H
