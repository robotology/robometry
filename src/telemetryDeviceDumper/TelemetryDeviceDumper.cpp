/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */
#include "TelemetryDeviceDumper.h"
#include <array>
#include <algorithm>
#include <cmath>

using namespace robometry;
using namespace yarp::os;
using namespace yarp::dev;

constexpr double log_thread_default{ 0.010 };


void convertVectorFromDegreesToRadians(std::vector<double>& inVec) {
    std::transform(inVec.begin(), inVec.end(), inVec.begin(), [](double el) -> double { return el * M_PI / 180.0; });
}

void addVectorOfStringToProperty(yarp::os::Property& prop, std::string key, std::vector<std::string>& list)
{
    prop.addGroup(key);
    yarp::os::Bottle& bot = prop.findGroup(key).addList();
    for (size_t i = 0; i < list.size(); i++)
    {
        bot.addString(list[i].c_str());
    }
    return;
}

bool getConfigParamsAsList(yarp::os::Searchable& config, std::string propertyName, std::vector<std::string>& list)
{
    yarp::os::Property prop;
    prop.fromString(config.toString().c_str());
    yarp::os::Bottle* propNames = prop.find(propertyName).asList();
    if (propNames == nullptr)
    {
        yError() << "telemetryDeviceDumper: Error parsing parameters: \" " << propertyName << " \" should be followed by a list\n";
        return false;
    }

    list.resize(propNames->size());
    for (auto elem = 0u; elem < propNames->size(); elem++)
    {
        list[elem] = propNames->get(elem).asString().c_str();
    }

    return true;
}

bool getUsedDOFsList(yarp::os::Searchable& config, std::vector<std::string>& usedDOFs)
{
    return getConfigParamsAsList(config, "axesNames", usedDOFs);
}

// For now also the period is harcoded, it can be configured
TelemetryDeviceDumper::TelemetryDeviceDumper() : yarp::os::PeriodicThread(log_thread_default) {
    // TODO
}

TelemetryDeviceDumper::~TelemetryDeviceDumper() {
    // TODO
}

bool TelemetryDeviceDumper::loadSettingsFromConfig(yarp::os::Searchable& config)
{

    yarp::os::Property prop;
    prop.fromString(config.toString().c_str());

    std::string logControlBoardQuantitiesOptionName = "logControlBoardQuantities";
    if (prop.check(logControlBoardQuantitiesOptionName.c_str())) {
        settings.logControlBoardQuantities = prop.find(logControlBoardQuantitiesOptionName.c_str()).asBool();
    }

    std::string logIEncodersOptionName = "logIEncoders";
    if (prop.check(logIEncodersOptionName.c_str())) {
        settings.logIEncoders = prop.find(logIEncodersOptionName.c_str()).asBool();
    }

    std::string logITorqueControlOptionName = "logITorqueControl";
    if (prop.check(logITorqueControlOptionName.c_str())) {
        settings.logITorqueControl = prop.find(logITorqueControlOptionName.c_str()).asBool();
    }

    std::string logIMotorEncodersOptionName = "logIMotorEncoders";
    if (prop.check(logIMotorEncodersOptionName.c_str())) {
        settings.logIMotorEncoders = prop.find(logIMotorEncodersOptionName.c_str()).asBool();
    }

    std::string logIControlModeOptionName = "logIControlMode";
    if (prop.check(logIControlModeOptionName.c_str())) {
        settings.logIControlMode = prop.find(logIControlModeOptionName.c_str()).asBool();
    }

    std::string logIInteractionModeOptionName = "logIInteractionMode";
    if (prop.check(logIInteractionModeOptionName.c_str())) {
        settings.logIInteractionMode = prop.find(logIInteractionModeOptionName.c_str()).asBool();
    }

    std::string logIPidControlOptionName = "logIPidControl";
    if (prop.check(logIPidControlOptionName.c_str())) {
        settings.logIPidControl = prop.find(logIPidControlOptionName.c_str()).asBool();
    }

    std::string logIAmplifierControlOptionName = "logIAmplifierControl";
    if (prop.check(logIAmplifierControlOptionName.c_str())) {
        settings.logIAmplifierControl = prop.find(logIAmplifierControlOptionName.c_str()).asBool();
    }

    std::string logILocalization2DOptionName = "logILocalization2D";
    if (prop.check(logILocalization2DOptionName.c_str())) {
        settings.logILocalization2D = prop.find(logILocalization2DOptionName.c_str()).asBool();
    }

    std::string logIRawValuesPublisherOptionName = "logIRawValuesPublisher";
    if (prop.check(logIRawValuesPublisherOptionName.c_str())) 
    {
        settings.logIRawValuesPublisher = prop.find(logIRawValuesPublisherOptionName.c_str()).asBool();
    }
    
    std::string useRadians = "useRadians";
    if (prop.check(useRadians.c_str())) {
        settings.useRadians = prop.find(useRadians.c_str()).asBool();
    }

    std::string saveBufferManagerConfiguration = "saveBufferManagerConfiguration";
    if (prop.check(saveBufferManagerConfiguration.c_str())) {
        settings.saveBufferManagerConfiguration = prop.find(saveBufferManagerConfiguration.c_str()).asBool();
    }

    std::string localizationRemoteName = "localizationRemoteName";
    if (prop.check(localizationRemoteName.c_str()) && prop.find(localizationRemoteName.c_str()).isString()) {
        settings.localizationRemoteName = prop.find(localizationRemoteName.c_str()).asString();
    }

    std::string rawValuesPublisherRemoteName = "rawValuesPublisherRemoteName";
    if (prop.check(rawValuesPublisherRemoteName.c_str()) && prop.find(rawValuesPublisherRemoteName.c_str()).isString()) {
        settings.rawValuesPublisherRemoteName = prop.find(rawValuesPublisherRemoteName.c_str()).asString();
    }

    // BufferManager options
    std::string json_file = "json_file";
    if (prop.check(json_file.c_str()) && prop.find(json_file.c_str()).isString()) {
        auto json_path = prop.find(json_file.c_str()).asString();
        bool ok = bufferConfigFromJson(m_bufferConfig, json_path);
        return ok;
    }
    else {
        std::string experimentName = "experimentName";
        if (prop.check(experimentName.c_str()) && prop.find(experimentName.c_str()).isString()) {
            m_bufferConfig.filename = prop.find(experimentName.c_str()).asString();
        }
        else {
            yError() << "TelemetryDeviceDumper: missing" << experimentName;
            return false;
        }

        std::string path = "path";
        if (prop.check(path.c_str()) && prop.find(path.c_str()).isString()) {
            m_bufferConfig.path = prop.find(path.c_str()).asString();
        }

        std::string n_samples = "n_samples";
        if (prop.check(n_samples.c_str()) && prop.find(n_samples.c_str()).isInt32()) {
            m_bufferConfig.n_samples = prop.find(n_samples.c_str()).asInt32();
        }
        else {
            yError() << "TelemetryDeviceDumper: missing" << n_samples;
            return false;
        }

        std::string save_periodically = "save_periodically";
        if (prop.check(save_periodically.c_str()) && prop.find(save_periodically.c_str()).isBool()) {
            m_bufferConfig.save_periodically = prop.find(save_periodically.c_str()).asBool();
        }

        if (m_bufferConfig.save_periodically) {
            std::string save_period = "save_period";
            if (prop.check(save_period.c_str()) && prop.find(save_period.c_str()).isFloat64()) {
                m_bufferConfig.save_period = prop.find(save_period.c_str()).asFloat64();
            }
            else {
                yError() << "TelemetryDeviceDumper: missing" << save_period;
                return false;
            }

            std::string data_threshold = "data_threshold";
            if (prop.check(data_threshold.c_str()) && prop.find(data_threshold.c_str()).isInt32()) {
                m_bufferConfig.data_threshold = prop.find(data_threshold.c_str()).asInt32();
            }

        }

        if (prop.check("log_period") && prop.find("log_period").isFloat64()) {
            this->setPeriod(prop.find("log_period").asFloat64());
        }

        std::string auto_save = "auto_save";
        if (prop.check(auto_save.c_str()) && prop.find(auto_save.c_str()).isBool()) {
            m_bufferConfig.auto_save = prop.find(auto_save.c_str()).asBool();
        }

        std::string yarp_robot_name = "yarp_robot_name";
        if (prop.check(yarp_robot_name.c_str()) && prop.find(yarp_robot_name.c_str()).isString()) {
            m_bufferConfig.yarp_robot_name = prop.find(yarp_robot_name.c_str()).asString();
        }

    }

    m_bufferConfig.mat_file_version = matioCpp::FileVersion::MAT7_3;

    if (!(m_bufferConfig.auto_save || m_bufferConfig.save_periodically)) {
        yError() << "TelemetryDeviceDumper: both auto_save and save_periodically are set to false, nothing will be saved.";
        return false;
    }

    return true;
}

bool TelemetryDeviceDumper::open(yarp::os::Searchable& config) {

    std::lock_guard<std::mutex> guard(this->deviceMutex);

    bool ok;

    // Load settings in the class
    ok = this->loadSettingsFromConfig(config);
    if (!ok)
    {
        yError() << "telemetryDeviceDumper: Problem in loading settings from config.";
        return false;
    }

    // Open Localization2DClient
    if (settings.logILocalization2D) {
        yarp::os::Property loc2DClientProp{{"device", Value("localization2DClient")},
                                           {"remote", Value(settings.localizationRemoteName)},
                                           {"local",  Value("/telemetryDeviceDumper" + settings.localizationRemoteName + "/client")}};
        ok = this->localization2DClient.open(loc2DClientProp);
        ok = ok && this->localization2DClient.view(iloc);
        if (!ok) {
            yError() << "telemetryDeviceDumper: Problem opening the localization2DClient.";
            return false;
        }
    }

    // Open the controlboard remapper
    ok = this->openRemapperControlBoard(config);
    if (!ok)
    {
        yError() << "telemetryDeviceDumper: Problem in opening controlboard remapper.";
        return false;
    }

    // Open RawValuesPublisherClient
    if (settings.logIRawValuesPublisher) {
        yarp::os::Property rawValPubClientProp{{"device", Value("rawValuesPublisherClient")},
                                           {"remote", Value(settings.rawValuesPublisherRemoteName)}, //must have the name of the remote port defined in the related nws, i.e. RawValuesPublisherServer
                                           {"local",  Value("/telemetryDeviceDumper" + settings.rawValuesPublisherRemoteName + "/client")}};
        ok = this->rawValuesPublisherClient.open(rawValPubClientProp);
        ok = ok && this->rawValuesPublisherClient.view(iravap);
        if (!ok) {
            yError() << "telemetryDeviceDumper: Problem opening the rawValuesPublisherClient.";
            return false;
        }
    }
    
    ok = this->configBufferManager(config);
    if (!ok)
    {
        yError() << "telemetryDeviceDumper: Problem in configuring the buffer manager.";
        return false;
    }

    return true;
}

bool TelemetryDeviceDumper::openRemapperControlBoard(yarp::os::Searchable& config)
{
    // Pass to the remapper just the relevant parameters (axesList)
    yarp::os::Property propRemapper;
    propRemapper.put("device", "controlboardremapper");
    bool ok = getUsedDOFsList(config, jointNames);
    if (!ok) return false;

    // Add the joint names to the BufferManager
    // Note that this will overwrite what is set from the configuration phase
    m_bufferConfig.description_list = jointNames;

    addVectorOfStringToProperty(propRemapper, "axesNames", jointNames);

    ok = remappedControlBoard.open(propRemapper);

    if (!ok)
    {
        return ok;
    }

    // View relevant interfaces for the remappedControlBoard
    ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.multwrap);
    // TODO: check if it has to be enabled by options
    if (settings.logControlBoardQuantities || settings.logIEncoders) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.encs);
    }
    if (settings.logControlBoardQuantities || settings.logIMotorEncoders) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.imotenc);
    }
    if (settings.logControlBoardQuantities || settings.logIPidControl) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.pid);
    }
    if (settings.logControlBoardQuantities || settings.logIAmplifierControl) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.amp);
    }
    if (settings.logControlBoardQuantities || settings.logIControlMode) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.cmod);
    }
    if (settings.logControlBoardQuantities || settings.logIInteractionMode) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.imod);
    }
    if (settings.logControlBoardQuantities || settings.logITorqueControl) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.itrq);
    }
    if (!ok)
    {
        yError() << "telemetryDeviceDumper: open impossible to use the necessary interfaces in remappedControlBoard";
        return ok;
    }

    int axes = 0;
    if (settings.logControlBoardQuantities){
        ok = ok && remappedControlBoardInterfaces.encs->getAxes(&axes);
        if (ok) {
            this->resizeBuffers(axes);
        }
        else {
            yError() << "telemetryDeviceDumper: open impossible to use the necessary interfaces in remappedControlBoard";
            return ok;
        }
    }

    return true;
}

bool TelemetryDeviceDumper::attachAllControlBoards(const yarp::dev::PolyDriverList& p_list) {
    PolyDriverList controlBoardList;
    for (size_t devIdx = 0; devIdx < (size_t)p_list.size(); devIdx++)
    {
        yarp::dev::IEncoders* pEncs = 0;
        if (p_list[devIdx]->poly->view(pEncs))
        {
            controlBoardList.push(const_cast<PolyDriverDescriptor&>(*p_list[devIdx]));
        }
    }

    // Attach the controlBoardList to the controlBoardRemapper
    bool ok = remappedControlBoardInterfaces.multwrap->attachAll(controlBoardList);

    if (!ok)
    {
        yError() << "telemetryDeviceDumper: attachAll in attachAll of the remappedControlBoard";
        return false;
    }

    return true;
}

void TelemetryDeviceDumper::resizeBuffers(int size) {
    // Let's resize all, we will see later.
    this->jointPos.resize(size);
    this->jointVel.resize(size);
    this->jointAcc.resize(size);
    // TODO see if it has to be enabled by options
    this->jointPosErr.resize(size);
    this->jointPosRef.resize(size);
    this->jointTrqErr.resize(size);
    this->jointTrqRef.resize(size);
    this->jointPWM.resize(size);
    this->jointCurr.resize(size);
    this->jointTrq.resize(size);
    this->motorEnc.resize(size);
    this->motorVel.resize(size);
    this->motorAcc.resize(size);
    this->controlModes.resize(size);
    this->interactionModes.resize(size);
    // OdometryData has 9 fields
    this->odometryData.resize(9);
    this->rawDataValuesMap.clear();

}

bool TelemetryDeviceDumper::configBufferManager(yarp::os::Searchable& conf) {
    bool ok{ true };

    if (ok && (settings.logIEncoders || settings.logControlBoardQuantities)) {
        ok = ok && bufferManager.addChannel({ "joints_state::positions", {jointPos.size(), 1}, m_bufferConfig.description_list, {"deg"} });
    }

    if (ok && (settings.logIEncoders || settings.logControlBoardQuantities)) {
        ok = ok && bufferManager.addChannel({ "joints_state::velocities", {jointVel.size(), 1}, m_bufferConfig.description_list, {"deg/s"} });
    }

    if (ok && (settings.logIEncoders || settings.logControlBoardQuantities)) {
        ok = ok && bufferManager.addChannel({ "joints_state::accelerations", {jointAcc.size(), 1}, m_bufferConfig.description_list, {"deg/s^2"} });
    }

    // TODO check if it is more convenient having more BM
    if (ok && (settings.logIPidControl || settings.logControlBoardQuantities)) {
        ok = ok && bufferManager.addChannel({ "PIDs::position_error", {jointPosErr.size(), 1}, m_bufferConfig.description_list });
        ok = ok && bufferManager.addChannel({ "PIDs::position_reference", {jointPosRef.size(), 1}, m_bufferConfig.description_list });
        ok = ok && bufferManager.addChannel({ "PIDs::torque_error", {jointTrqErr.size(), 1}, m_bufferConfig.description_list });
        ok = ok && bufferManager.addChannel({ "PIDs::torque_reference", {jointTrqRef.size(), 1}, m_bufferConfig.description_list });
    }
    if (ok && (settings.logIAmplifierControl || settings.logControlBoardQuantities)) {
        ok = ok && bufferManager.addChannel({ "motors_state::PWM", {jointPWM.size(), 1}, m_bufferConfig.description_list, {"V"} });
        ok = ok && bufferManager.addChannel({ "motors_state::currents", {jointCurr.size(), 1}, m_bufferConfig.description_list, {"mA"} });
    }
    if (ok && (settings.logITorqueControl || settings.logControlBoardQuantities)) {
        ok = ok && bufferManager.addChannel({ "joints_state::torques", {jointTrq.size(), 1}, m_bufferConfig.description_list, {"Nm"} });
    }
    if (ok && (settings.logIMotorEncoders || settings.logControlBoardQuantities)) {
        ok = ok && bufferManager.addChannel({ "motors_state::positions", {motorEnc.size(), 1}, m_bufferConfig.description_list });
        ok = ok && bufferManager.addChannel({ "motors_state::velocities", {motorVel.size(), 1}, m_bufferConfig.description_list });
        ok = ok && bufferManager.addChannel({ "motors_state::accelerations", {motorAcc.size(), 1}, m_bufferConfig.description_list });
    }

    if (ok && (settings.logIControlMode || settings.logControlBoardQuantities)) {
        ok = ok && bufferManager.addChannel({ "joints_state::control_mode", {controlModes.size(), 1}, m_bufferConfig.description_list });
    }
    if (ok && (settings.logIInteractionMode || settings.logControlBoardQuantities)) {
        ok = ok && bufferManager.addChannel({ "joints_state::interaction_mode", {interactionModes.size(), 1}, m_bufferConfig.description_list });
    }

    if (ok && (settings.logILocalization2D)) {
        ok = ok && bufferManager.addChannel({ "odometry_data", {odometryData.size(), 1}, m_bufferConfig.description_list });
    }

    // TODO check if we have nr of channels ~= 0
    return ok;
}

bool TelemetryDeviceDumper::attachAll(const yarp::dev::PolyDriverList& device2attach) {
    std::lock_guard<std::mutex> guard(this->deviceMutex);

    bool ok = true;
    ok = ok && this->attachAllControlBoards(device2attach);

    if (ok && (settings.logIRawValuesPublisher))
    {
        // COnfiguring channels using metadata from interfaces
        rawValuesKeyMetadataMap metadata = {}; // I just need to call it once while configuring (I think) 
        iravap->getMetadataMAP(metadata);
        for (auto [k, m] : metadata.metadataMap)
        {
            ok = ok && bufferManager.addChannel({ "raw_data_values::"+k, {static_cast<uint16_t>(m.size), 1}, m.rawValueNames });
        }
    }
    
    ok = ok && bufferManager.configure(m_bufferConfig);
    
    if (ok)
    {
        correctlyConfigured = true;
        this->start();
    }

    return ok;
}
bool TelemetryDeviceDumper::detachAll()
{
    std::lock_guard<std::mutex> guard(this->deviceMutex);
    correctlyConfigured = false;
    if (isRunning())
    {
        stop();
    }
    return  this->remappedControlBoardInterfaces.multwrap->detachAll();
}

bool TelemetryDeviceDumper::close()
{
    correctlyConfigured = false;
    remappedControlBoard.close();
    if (settings.logILocalization2D) {
        localization2DClient.close();
    }

    if (settings.logIRawValuesPublisher)
    {
        rawValuesPublisherClient.close();
    }
    

    bool ok = true;
    if (settings.saveBufferManagerConfiguration) {
        auto buffConfToSave = bufferManager.getBufferConfig();
        ok = bufferConfigToJson(buffConfToSave, buffConfToSave.path + "bufferConfig" + buffConfToSave.filename + ".json");
    }

    return ok;
}


void TelemetryDeviceDumper::readSensors()
{
    bool ok;
    // Read encoders
    if (settings.logIEncoders || settings.logControlBoardQuantities) {
        sensorsReadCorrectly = remappedControlBoardInterfaces.encs->getEncoders(jointPos.data());
        if (!sensorsReadCorrectly)
        {
            yWarning() << "telemetryDeviceDumper warning : joint positions was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointPos, "joints_state::positions");
        }
    }

    // At the moment we are assuming that all joints are revolute

    if (settings.logIEncoders || settings.logControlBoardQuantities)
    {
        ok = remappedControlBoardInterfaces.encs->getEncoderSpeeds(jointVel.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint velocities was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointVel , "joints_state::velocities");
        }

    }

    if (settings.logIEncoders || settings.logControlBoardQuantities)
    {
        ok = remappedControlBoardInterfaces.encs->getEncoderAccelerations(jointAcc.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint accelerations was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointAcc, "joints_state::accelerations");
        }


    }

    // Read PID
    if (settings.logIPidControl || settings.logControlBoardQuantities) {
        ok = remappedControlBoardInterfaces.pid->getPidErrors(VOCAB_PIDTYPE_POSITION, jointPosErr.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint position errors was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointPosErr, "PIDs::position_error");
        }

        ok = remappedControlBoardInterfaces.pid->getPidReferences(VOCAB_PIDTYPE_POSITION, jointPosRef.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint position references was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointPosRef, "PIDs::position_reference");
        }


        ok = remappedControlBoardInterfaces.pid->getPidErrors(VOCAB_PIDTYPE_TORQUE, jointTrqErr.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint torque errors was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointPosErr, "PIDs::torque_error");
        }

        ok = remappedControlBoardInterfaces.pid->getPidReferences(VOCAB_PIDTYPE_TORQUE, jointTrqRef.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint torque references was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointTrqRef, "PIDs::torque_reference");
        }
    }
    // Read amplifier
    if (settings.logIAmplifierControl || settings.logControlBoardQuantities) {
        for (int j = 0; j < jointPWM.size(); j++)
        {
            ok &= remappedControlBoardInterfaces.amp->getPWM(j, &jointPWM[j]);
        }
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : voltage PWM was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointPWM, "motors_state::PWM");
        }

        ok = remappedControlBoardInterfaces.amp->getCurrents(jointCurr.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : current was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointCurr, "motors_state::currents");
        }
    }
    // Read torque
    if (settings.logITorqueControl || settings.logControlBoardQuantities) {
        ok = remappedControlBoardInterfaces.itrq->getTorques(jointTrq.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : torque was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointTrq, "joints_state::torques");
        }
    }

    // Read motor
    if (settings.logIMotorEncoders || settings.logControlBoardQuantities) {
        ok = remappedControlBoardInterfaces.imotenc->getMotorEncoders(motorEnc.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : motor encoder was not readed correctly";
        }
        else
        {
            bufferManager.push_back(motorEnc, "motors_state::positions");
        }

        ok = remappedControlBoardInterfaces.imotenc->getMotorEncoderSpeeds(motorVel.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : motor velocity was not readed correctly";
        }
        else
        {
            bufferManager.push_back(motorVel, "motors_state::velocities");
        }

        ok = remappedControlBoardInterfaces.imotenc->getMotorEncoderAccelerations(motorAcc.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : motor acceleration was not readed correctly";
        }
        else
        {
            bufferManager.push_back(motorAcc, "motors_state::accelerations");
        }
    }

    // Read modes
    if (settings.logIControlMode || settings.logControlBoardQuantities) {
        for (int i = 0; i < interactionModes.size(); i++)
        {
            int tmp;
            ok &= remappedControlBoardInterfaces.cmod->getControlMode(i, &tmp);
            controlModes[i] = (double)tmp;
        }
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : control modes wa not readed correctly";
        }
        else
        {
            bufferManager.push_back(controlModes, "joints_state::control_mode");
        }
    }

    if (settings.logIInteractionMode || settings.logControlBoardQuantities) {
        for (int i = 0; i < interactionModes.size(); i++)
        {
            yarp::dev::InteractionModeEnum tmp;
            ok &= remappedControlBoardInterfaces.imod->getInteractionMode(i, &tmp);
            if (!ok) {
                break;
            }
            if (tmp == VOCAB_IM_STIFF)          interactionModes[i] = 0.0;
            else if (tmp == VOCAB_IM_COMPLIANT) interactionModes[i] = 1.0;
            else                                interactionModes[i] = -1.0;
        }
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : interaction mode was not readed correctly";
        }
        else
        {
            bufferManager.push_back(interactionModes, "joints_state::interaction_mode");
        }
    }

    if (settings.useRadians) {
        // TODO check if it is safe to call transform on empty vectors.
        // TODO handle prismatic joints.
        convertVectorFromDegreesToRadians(jointPos);
        convertVectorFromDegreesToRadians(jointVel);
        convertVectorFromDegreesToRadians(jointAcc);
        convertVectorFromDegreesToRadians(jointPosErr);
    }

}

void TelemetryDeviceDumper::readOdometryData() {
    bool ok;
    yarp::dev::OdometryData yarpOdomData;
    ok = iloc->getEstimatedOdometry(yarpOdomData);
    if (!ok)
    {
        yWarning() << "telemetryDeviceDumper warning : odometry_data was not read correctly";
    }
    else
    {
        odometryData[0] = yarpOdomData.odom_x;     odometryData[1] = yarpOdomData.odom_y;     odometryData[2] = yarpOdomData.odom_theta;
        odometryData[3] = yarpOdomData.base_vel_x; odometryData[4] = yarpOdomData.base_vel_y; odometryData[5] = yarpOdomData.base_vel_theta;
        odometryData[6] = yarpOdomData.odom_vel_x; odometryData[7] = yarpOdomData.odom_vel_y; odometryData[8] = yarpOdomData.odom_vel_theta;
        bufferManager.push_back(odometryData, "odometry_data");
    }
}

void TelemetryDeviceDumper::readRawValuesData()
{
    bool ok;
    ok = iravap->getRawDataMap(rawDataValuesMap);
    if (!ok)
    {
        yWarning() << "telemetryDeviceDumper warning : raw_data_values was not read correctly";
    }
    else
    {
        for (auto [key,value] : rawDataValuesMap)
        {
            bufferManager.push_back(value, "raw_data_values::"+key);
        }
    }

}

void TelemetryDeviceDumper::run() {
    if (correctlyConfigured) {
        readSensors();
        if (settings.logILocalization2D) {
            readOdometryData();
        }

        if (settings.logIRawValuesPublisher)
        {
            readRawValuesData();
        }
        
    }
    return;
}

