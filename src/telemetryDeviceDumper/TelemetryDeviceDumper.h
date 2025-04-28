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
#include <iCub/IRawValuesPublisher.h>
#include <yarp/dev/IMotor.h>
#include <yarp/dev/ITorqueControl.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/PeriodicThread.h>
#include <robometry/BufferManager.h>

#include <unordered_map>
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

namespace robometry {


struct TelemetryDeviceDumperSettings {

    bool logControlBoardQuantities{ false };
    bool logIEncoders{ true };
    bool logITorqueControl{ false };
    bool logIMotorEncoders{ false };
    bool logIControlMode{ false };
    bool logIInteractionMode{ false };
    bool logIPidControl{ false };
    bool logIAmplifierControl{ false };
    bool logIMotorTemperatures{ false };
    bool logILocalization2D{ false };
    bool logIRawValuesPublisher { false };
    bool useRadians{ false };
    bool saveBufferManagerConfiguration{ false };
    std::string localizationRemoteName{ "" };
    std::string rawValuesPublisherRemoteName { "" };

};
/**
 * @brief The `telemetryDeviceDumper` is a [yarp device](http://yarp.it/git-master/note_devices.html)
 * that has to be launched through the [`yarprobotinterface`](http://yarp.it/git-master/yarprobotinterface.html)
 * for dumping quantities from your robot(for example encoders, velocities etc) in base of what specified in the configuration.
 *
 * @section Params Parameters
 * | Parameter name | Type | Units | Default | Required | Description |
 * | -------- | -------- | -------- | -------- | -------- | -------- |
 * | `axesNames`     | List of strings     | -  | -     | Yes     | The axes contained in the axesNames parameter are then mapped to the wrapped controlboard in the attachAll method, using controlBoardRemapper class. |
 * | `logIEncoders`     | bool     | -     |  true | No     | Enable the log of `joints_state::positions`, `joints_state::velocities` and `joints_state::accelerations` (http://yarp.it/git-master/classyarp_1_1dev_1_1IEncoders.html)     |
 * | `logITorqueControl`     | bool     | -     | false     | No     | Enable the log of `joints_state::torques`(http://yarp.it/git-master/classyarp_1_1dev_1_1ITorqueControl.html).     |
 * | `logIMotorEncoders`     | bool     | -     | false     | No     | Enable the log of `motors_state::positions`, `motors_state::velocities` and `motors_state::accelerations` (http://yarp.it/git-master/classyarp_1_1dev_1_1IMotorEncoders.html).     |
 * | `logIControlMode`     | bool     | -     | false     | No     | Enable the log of `joints_state::control_mode` (http://yarp.it/git-master/classyarp_1_1dev_1_1IControlMode.html.     |
 * | `logIInteractionMode`     | bool     | -     | false     | No     | Enable the log of `joints_state::interaction_mode` (http://yarp.it/git-master/classyarp_1_1dev_1_1IInteractionMode.html.     |
 * | `logIPidControl`     | bool     | -     | false     | No     | Enable the log of `PIDs::position_error`, `PIDs::position_reference`, `PIDs::torque_error`, `PIDs::torque_reference`(http://yarp.it/git-master/classyarp_1_1dev_1_1IPidControl.html).|
 * | `logIAmplifierControl`     | bool     | -     | false     | No     | Enable the log of `motors_state::pwm` and `motors_state::currents` (http://yarp.it/git-master/classyarp_1_1dev_1_1IAmplifierControl.html).     |
 * | `logIMotorTemperatures`     | bool     | -     | false     | No     | Enable the log of `motors_state::temperatures` available only with [yarp branch](https://github.com/ami-iit/yarp/tree/yarp-3.10.1-motor-temperature) (http://yarp.it/git-master/classyarp_1_1dev_1_1IMotor.html).     |
 * | `logControlBoardQuantities` | bool     | -     | false     | No     | Enable the log of all the quantities that requires the attach to a control board (`logIEncoders`, `logITorqueControl`, `logIMotorEncoders`, `logIControlMode`, `logIInteractionMode`, `logIPidControl`, `logIAmplifierControl`). |
 * | `logILocalization2D` | bool     | -     | false     | No     | Enable the log of `odometry_data` (http://yarp.it/git-master/classyarp_1_1dev_1_1Nav2D_1_1ILocalization2D.html). |
 * | `logIRawValuesPublisher` | bool |   -   | false    | No | Enable the log of `raw values` (https://github.com/robotology/icub-main/blob/devel/src/libraries/iCubDev/include/iCub/IRawValuesPublisher.h) |
 * | `saveBufferManagerConfiguration`     | bool     | -    | false     | No     | Enable the save of the configuration of the BufferManager into `path`+ `"bufferConfig"` + `experimentName` + `".json"`     |
 * | `json_file`     | string     | -     | -     | No     | Configure the `robometry::BufferManager`s reading from a json file like [in Example configuration file](#example-configuration-file). Note that this configuration will overwrite the parameter-by-parameter configuration   |
 * | `experimentName`     | string     | -     | -     | Yes     | Prefix of the files that will be saved. The files will be named: `experimentName`+`timestamp`+ `".mat"`.     |
 * | `path`     | string     | -     | -     | No     | Path of the folder where the data will be saved.     |
 * | `n_samples`     | size_t     | -     | -     | Yes     | The max number of samples contained in the circular buffer/s     |
 * | `save_periodically`     | bool     | -     | false     | No(but it has to be set to true if `auto_save` is set to false)     | The flag for enabling the periodic save thread.     |
 * | `save_period`     | double     | seconds     | -     | Yes(if `save_periodically` is set to true)     | The period in seconds of the save thread     |
 * | `log_period`     | double     | seconds     | 0.010    | No    | The period in seconds of the logging thread.     |
 * | `data_threshold`     | size_t     | -     | 0     | No     | The save thread saves to a file if there are at least `data_threshold` samples     |
 * | `auto_save`     | bool     | -     | false     | No(but it has to be set to true if `save_periodically` is set to false)     | the flag for enabling the save in the destructor of the `robometry::BufferManager`     |
 * | `yarp_robot_name`     | string     | -     | ""     | No     | Name of the robot used during the experiment.     |
 *
 * @section Mapping_mat_YARP Mapping .mat variables -> YARP interfaces
 *
 * | Variable name        | YARP interface |
 * | -------------------- | -------------- |
 * | `joints_state::positions`           | [`yarp::dev::IEncoders::getEncoders`](http://yarp.it/git-master/classyarp_1_1dev_1_1IEncoders.html#abcfe10041280b99c7c4384c4fd93a9dd) |
 * | `joints_state::velocities`           | [`yarp::dev::IEncoders::getEncoderSpeeds`](http://yarp.it/git-master/classyarp_1_1dev_1_1IEncoders.html#ac84ae2f65f4a93b66827a3a424d1f743) |
 * | `joints_state::accelerations`       | [`yarp::dev::IEncoders::getEncoderAccelerations`](http://yarp.it/git-master/classyarp_1_1dev_1_1IEncoders.html#a3bcb5fe5c6a5e15e57f4723bbe12c57a) |
 * | `joints_state::torques`       | [`yarp::dev::ITorqueControl`](https://yarp.it//git-master/classyarp_1_1dev_1_1ITorqueControl.html#a45c1ad295b7fff91005a9f4564a35b2a)|
 * | `motors_state::positions`     | [`yarp::dev::IMotorEncoders::getMotorEncoders`](http://yarp.it/git-master/classyarp_1_1dev_1_1IMotorEncoders.html#ac02fb05bb3e9ac9a381d41b59c38c412) |
 * | `motors_state::velocities`     | [`yarp::dev::IMotorEncoders::getMotorEncoderSpeeds`](http://yarp.it/git-master/classyarp_1_1dev_1_1IMotorEncoders.html#a4624348cc129bfeb12ad0e6d2892b76c) |
 * | `motors_state::accelerations` | [`yarp::dev::IMotorEncoders::getMotorEncoderAccelerations`](http://yarp.it/git-master/classyarp_1_1dev_1_1IMotorEncoders.html#a9394d8b5cc4f3d58aeaa07c3fb9a6e6a) |
 * | `motors_state::pwm` | [`yarp::dev::IAmplifierControl::getPWM`](https://yarp.it//git-master/classyarp_1_1dev_1_1IAmplifierControl.html#a71ab30ccf182387bf6552d74f64ccfa5) |
 * | `motors_state::currents` | [`yarp::dev::IAmplifierControl::getCurrents`](https://yarp.it//git-master/classyarp_1_1dev_1_1IAmplifierControl.html#a60ab9c4fdc7f81bd136ad246a9dc57e8) |
 * | `motors_state::temperatures` | [`yarp::dev::IMotor::getTemperatures`](https://yarp.it//git-master/classyarp_1_1dev_1_1IMotor.html#a60ab9c4fdc7f81bd136ad246a9dc57e8) |
 * | `joints_state::control_mode`       | [`yarp::dev::IControlMode::getControlModes`](http://yarp.it/git-master/classyarp_1_1dev_1_1IControlMode.html#a32f04715873a8099ec40671f65faff8d) |
 * | `joints_state::interaction_mode`   | [`yarp::dev::IInteractionMode::getInteractionModes`](http://yarp.it/git-master/classyarp_1_1dev_1_1IInteractionMode.html#a6055ce20216f479da6c63807a4d11f54) |
 * | `PIDs::position_error`     | [`yarp::dev::IPidControl::getPidErrors`](http://yarp.it/git-master/classyarp_1_1dev_1_1IPidControl.html#aea29e0fdf34f819ac69a3b940556ba28) |
 * | `PIDs::position_reference` | [`yarp::dev::IPidControl::getPidReferences`](http://yarp.it/git-master/classyarp_1_1dev_1_1IPidControl.html#a29e8f684a15d859229a9ae2902f886da) |
 * | `PIDs::torque_error`       | [`yarp::dev::IPidControl::getPidErrors`](http://yarp.it/git-master/classyarp_1_1dev_1_1IPidControl.html#aea29e0fdf34f819ac69a3b940556ba28) |
 * | `PIDs::torque_reference`   | [`yarp::dev::IPidControl::getPidReferences`](http://yarp.it/git-master/classyarp_1_1dev_1_1IPidControl.html#a29e8f684a15d859229a9ae2902f886da) |
 * | `PIDs::odometry_data   `   | [`yarp::dev::Nav2D::ILocalization2D::getEstimatedOdometry`](http://yarp.it/git-master/classyarp_1_1dev_1_1Nav2D_1_1ILocalization2D.html#a02bff57282777ce7511b671abd4c95f0) |
 * | `raw_data_values`          | [`iCub::debugLibrary::IRawValuesPublisher::getRawDataMap`] (https://github.com/robotology/icub-main/blob/devel/src/libraries/iCubDev/include/iCub/IRawValuesPublisher.h)
 * 
 * @section Example_xml Example of xml
 *
 * Example of xml file for using it on the `iCub` robot:
 *
 * @code{.xml}
 * <?xml version="1.0" encoding="UTF-8" ?>
 * <!DOCTYPE devices PUBLIC "-//YARP//DTD yarprobotinterface 3.0//EN" "http://www.yarp.it/DTD/yarprobotinterfaceV3.0.dtd">
 *
 *
 *     <device xmlns:xi="http://www.w3.org/2001/XInclude" name="telemetryDeviceDumper" type="telemetryDeviceDumper">
 *         <param name="axesNames">(torso_pitch,torso_roll,torso_yaw,neck_pitch, neck_roll,neck_yaw,l_shoulder_pitch,l_shoulder_roll,l_shoulder_yaw,l_elbow,l_wrist_prosup,l_wrist_pitch,l_wrist_yaw,r_shoulder_pitch,r_shoulder_roll,r_shoulder_yaw,r_elbow,r_wrist_prosup,r_wrist_pitch,r_wrist_yaw,l_hip_pitch,l_hip_roll,l_hip_yaw,l_knee,l_ankle_pitch,l_ankle_roll,r_hip_pitch,r_hip_roll,r_hip_yaw,r_knee,r_ankle_pitch,r_ankle_roll)</param>
 *         <param name="logIEncoders">true</param>
 *         <param name="logITorqueControl">true</param>
 *         <param name="logIMotorEncoders">true</param>
 *         <param name="logIControlMode">true</param>
 *         <param name="logIInteractionMode">true</param>
 *         <param name="logIPidControl">false</param>
 *         <param name="logIAmplifierControl">true</param>
 *         <param name="logIMotorTemperatures">false</param>
 *         <param name="logIRawValuesPublisher">false</param>
 *         <param name="saveBufferManagerConfiguration">true</param>
 *         <param name="experimentName">test_telemetry</param>
 *         <param name="path">/home/icub/test_telemetry/</param>
 *         <param name="n_samples">100000</param>
 *         <param name="save_periodically">true</param>
 *         <param name="save_period">120.0</param>
 *         <param name="log_period">0.010</param>
 *         <param name="data_threshold">300</param>
 *         <param name="auto_save">true</param>
 *
 *         <action phase="startup" level="15" type="attach">
 *             <paramlist name="networks">
 *                 <!-- motorcontrol and virtual torque sensors -->
 *                 <elem name="left_lower_leg">left_leg-eb7-j4_5-mc</elem>
 *                 <elem name="right_lower_leg">right_leg-eb9-j4_5-mc</elem>
 *                 <elem name="left_upper_leg">left_leg-eb6-j0_3-mc</elem>
 *                 <elem name="right_upper_leg">right_leg-eb8-j0_3-mc</elem>
 *                 <elem name="torso">torso-eb5-j0_2-mc</elem>
 *                 <elem name="right_lower_arm">right_arm-eb27-j4_7-mc</elem>
 *                 <elem name="left_lower_arm">left_arm-eb24-j4_7-mc</elem>
 *                 <elem name="right_upper_arm">right_arm-eb3-j0_3-mc</elem>
 *                 <elem name="left_upper_arm">left_arm-eb1-j0_3-mc</elem>
 *                 <elem name="head-j0">head-eb20-j0_1-mc</elem>
 *                 <elem name="head-j2">head-eb21-j2_5-mc</elem>
 *                 <!-- ft -->
 *             </paramlist>
 *         </action>
 *
 *         <action phase="shutdown" level="2" type="detach" />
 *
 *     </device>
 * @endcode
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
    void readRawValuesData();
    void resizeBuffers(int size);
    bool configBufferManager(yarp::os::Searchable& config);
    /** Remapped controlboard containg the axes for which the joint torques are estimated */
    yarp::dev::PolyDriver remappedControlBoard, localization2DClient, rawValuesPublisherClient;
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
        yarp::dev::IMotor* imot{ nullptr };
    } remappedControlBoardInterfaces;

    yarp::dev::Nav2D::ILocalization2D* iloc{nullptr};

    iCub::debugLibrary::IRawValuesPublisher* iravap{ nullptr };

    std::mutex deviceMutex;
    std::atomic<bool> correctlyConfigured{ false }, sensorsReadCorrectly{false};
    std::vector<double> jointPos, jointVel, jointAcc, jointPosErr, jointPosRef,
                        jointTrqErr, jointTrqRef, jointPWM, jointCurr, jointTrq,
                        motorEnc, motorVel, motorAcc, motorTemp, controlModes, interactionModes,
                        odometryData;

    std::map<std::string, std::vector<std::int32_t>> rawDataValuesMap;

    std::vector<std::string> jointNames;
    TelemetryDeviceDumperSettings settings;
    robometry::BufferConfig m_bufferConfig;
    robometry::BufferManager bufferManager;


};
}

#endif // ROBOMETRY_TELEMETRYDEVICEDUMPER_H
