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

using namespace yarp::telemetry::experimental;
using namespace yarp::os;
using namespace yarp::dev;

constexpr double period_thread{ 0.010 };


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
TelemetryDeviceDumper::TelemetryDeviceDumper() : yarp::os::PeriodicThread(period_thread) {
    // TODO
}

TelemetryDeviceDumper::~TelemetryDeviceDumper() {
    // TODO
}

bool TelemetryDeviceDumper::loadSettingsFromConfig(yarp::os::Searchable& config)
{

    yarp::os::Property prop;
    prop.fromString(config.toString().c_str());

    std::string useJointVelocityOptionName = "logJointVelocity";
    if (prop.check(useJointVelocityOptionName.c_str())) {
        yWarning() << "telemetryDeviceDumper: logJointVelocity is deprecated, use logEncoderQuantities instead.";
        settings.logJointVelocity = prop.find(useJointVelocityOptionName.c_str()).asBool();
    }

    std::string useJointAccelerationOptionName = "logJointAcceleration";
    if (prop.check(useJointAccelerationOptionName.c_str())) {
        yWarning() << "telemetryDeviceDumper: logJointAcceleration is deprecated, use logEncoderQuantities instead.";
        settings.logJointAcceleration = prop.find(useJointAccelerationOptionName.c_str()).asBool();
    }

    std::string logAllQuantitiesOptionName = "logAllQuantities";
    if (prop.check(logAllQuantitiesOptionName.c_str())) {
        settings.logAllQuantities = prop.find(logAllQuantitiesOptionName.c_str()).asBool();
    }

    std::string logEncoderQuantitiesOptionName = "logEncoderQuantities";
    if (prop.check(logEncoderQuantitiesOptionName.c_str())) {
        settings.logEncoderQuantities = prop.find(logEncoderQuantitiesOptionName.c_str()).asBool();
    }

    std::string logTorqueQuantitiesOptionName = "logTorqueQuantities";
    if (prop.check(logTorqueQuantitiesOptionName.c_str())) {
        settings.logTorqueQuantities = prop.find(logTorqueQuantitiesOptionName.c_str()).asBool();
    }

    std::string logMotorQuantitiesOptionName = "logMotorQuantities";
    if (prop.check(logMotorQuantitiesOptionName.c_str())) {
        settings.logMotorQuantities = prop.find(logMotorQuantitiesOptionName.c_str()).asBool();
    }

    std::string logModesQuantitiesOptionName = "logModesQuantities";
    if (prop.check(logModesQuantitiesOptionName.c_str())) {
        settings.logModesQuantities = prop.find(logModesQuantitiesOptionName.c_str()).asBool();
    }

    std::string logPIDQuantitiesOptionName = "logPIDQuantities";
    if (prop.check(logPIDQuantitiesOptionName.c_str())) {
        settings.logPIDQuantities = prop.find(logPIDQuantitiesOptionName.c_str()).asBool();
    }

    std::string logAmplifierQuantitiesOptionName = "logAmplifierQuantities";
    if (prop.check(logAmplifierQuantitiesOptionName.c_str())) {
        settings.logAmplifierQuantities = prop.find(logAmplifierQuantitiesOptionName.c_str()).asBool();
    }

    std::string useRadians = "useRadians";
    if (prop.check(useRadians.c_str())) {
        settings.useRadians = prop.find(useRadians.c_str()).asBool();
    }

    std::string saveBufferManagerConfiguration = "saveBufferManagerConfiguration";
    if (prop.check(saveBufferManagerConfiguration.c_str())) {
        settings.saveBufferManagerConfiguration = prop.find(saveBufferManagerConfiguration.c_str()).asBool();
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

        std::string auto_save = "auto_save";
        if (prop.check(auto_save.c_str()) && prop.find(auto_save.c_str()).isBool()) {
            m_bufferConfig.auto_save = prop.find(auto_save.c_str()).asBool();
        }

    }

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

    // Open the controlboard remapper
    ok = this->openRemapperControlBoard(config);
    if (!ok)
    {
        yError() << "telemetryDeviceDumper: Problem in opening controlboard remapper.";
        return false;
    }

    ok = this->configBufferManager(config);
    if (!ok)
    {
        yError() << "telemetryDeviceDumper: Problem in configuring the buffer manager.";
        return false;
    }

    return true;
}

bool TelemetryDeviceDumper::openRemapperControlBoard(os::Searchable& config)
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
    if (settings.logAllQuantities || settings.logEncoderQuantities
        || settings.logJointVelocity || settings.logJointAcceleration ) { // deprecated since v0.1.0
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.encs);
    }
    if (settings.logAllQuantities || settings.logMotorQuantities) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.imotenc);
    }
    if (settings.logAllQuantities || settings.logPIDQuantities) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.pid);
    }
    if (settings.logAllQuantities || settings.logAmplifierQuantities) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.amp);
    }
    if (settings.logAllQuantities || settings.logModesQuantities) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.cmod);
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.imod);
    }
    if (settings.logAllQuantities || settings.logTorqueQuantities) {
        ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.itrq);
    }
    if (!ok)
    {
        yError() << "telemetryDeviceDumper: open impossible to use the necessary interfaces in remappedControlBoard";
        return ok;
    }

    int axes = 0;
    ok = ok && remappedControlBoardInterfaces.encs->getAxes(&axes);
    if (ok) {
        this->resizeBuffers(axes);
    }
    else {
        yError() << "telemetryDeviceDumper: open impossible to use the necessary interfaces in remappedControlBoard";
        return ok;
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

}

bool TelemetryDeviceDumper::configBufferManager(yarp::os::Searchable& conf) {
    bool ok{ true };

    if (ok && (settings.logEncoderQuantities || settings.logAllQuantities)) {
        ok = ok && bufferManager.addChannel({ "encoders", {jointPos.size(), 1} });
    }
    
    if (ok && (settings.logJointVelocity || settings.logEncoderQuantities || settings.logAllQuantities)) {
        ok = ok && bufferManager.addChannel({ "velocity", {jointVel.size(), 1} });
    }

    if (ok && (settings.logJointAcceleration || settings.logEncoderQuantities || settings.logAllQuantities)) {
        ok = ok && bufferManager.addChannel({ "acceleration", {jointAcc.size(), 1} });
    }

    // TODO check if it is more convenient having more BM
    if (ok && (settings.logPIDQuantities || settings.logAllQuantities)) {
        ok = ok && bufferManager.addChannel({ "position_error", {jointPosErr.size(), 1} });
        ok = ok && bufferManager.addChannel({ "position_reference", {jointPosRef.size(), 1} });
        ok = ok && bufferManager.addChannel({ "torque_error", {jointTrqErr.size(), 1} });
        ok = ok && bufferManager.addChannel({ "torque_reference", {jointTrqRef.size(), 1} });
    }
    if (ok && (settings.logAmplifierQuantities || settings.logAllQuantities)) {
        ok = ok && bufferManager.addChannel({ "pwm", {jointPWM.size(), 1} });
        ok = ok && bufferManager.addChannel({ "current", {jointCurr.size(), 1} });
    }
    if (ok && (settings.logTorqueQuantities || settings.logAllQuantities)) {
        ok = ok && bufferManager.addChannel({ "torque", {jointTrq.size(), 1} });
    }
    if (ok && (settings.logMotorQuantities || settings.logAllQuantities)) {
        ok = ok && bufferManager.addChannel({ "motor_encoder", {motorEnc.size(), 1} });
        ok = ok && bufferManager.addChannel({ "motor_velocity", {motorVel.size(), 1} });
        ok = ok && bufferManager.addChannel({ "motor_accelerarion", {motorAcc.size(), 1} });
    }
    // TODO check if it better convert int -> double
    if (ok && (settings.logModesQuantities || settings.logAllQuantities)) {
        ok = ok && bufferManager_modes.addChannel({ "control_mode", {controlModes.size(), 1} });
        ok = ok && bufferManager_modes.addChannel({ "interaction_mode", {interactionModes.size(), 1} });
    }

    ok = ok && bufferManager.configure(m_bufferConfig);
    ok = ok && bufferManager_modes.configure(m_bufferConfig);
    // TODO check if we have nr of channels ~= 0
    return ok;
}

bool TelemetryDeviceDumper::attachAll(const yarp::dev::PolyDriverList& device2attach) {
    std::lock_guard<std::mutex> guard(this->deviceMutex);

    bool ok = true;
    ok = ok && this->attachAllControlBoards(device2attach);

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
    // Flush all the remaining data.
    bufferManager.saveToFile();
    bufferManager_modes.saveToFile();
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
    if (settings.logEncoderQuantities || settings.logAllQuantities) {
        sensorsReadCorrectly = remappedControlBoardInterfaces.encs->getEncoders(jointPos.data());
        if (!sensorsReadCorrectly)
        {
            yWarning() << "telemetryDeviceDumper warning : joint positions was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointPos, "encoders");
        }
    }

    // At the moment we are assuming that all joints are revolute

    if (settings.logJointVelocity || settings.logEncoderQuantities || settings.logAllQuantities)
    {
        ok = remappedControlBoardInterfaces.encs->getEncoderSpeeds(jointVel.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint velocities was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointVel , "velocity");
        }

    }

    if (settings.logJointAcceleration || settings.logEncoderQuantities || settings.logAllQuantities)
    {
        ok = remappedControlBoardInterfaces.encs->getEncoderAccelerations(jointAcc.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint accelerations was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointAcc, "acceleration");
        }


    }
    
    // Read PID
    if (settings.logPIDQuantities || settings.logAllQuantities) {
        ok = remappedControlBoardInterfaces.pid->getPidErrors(VOCAB_PIDTYPE_POSITION, jointPosErr.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint position errors was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointPosErr, "potition_error");
        }

        ok = remappedControlBoardInterfaces.pid->getPidReferences(VOCAB_PIDTYPE_POSITION, jointPosRef.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint position references was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointPosRef, "potition_reference");
        }


        ok = remappedControlBoardInterfaces.pid->getPidErrors(VOCAB_PIDTYPE_TORQUE, jointTrqErr.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint torque errors was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointPosErr, "torque_error");
        }

        ok = remappedControlBoardInterfaces.pid->getPidReferences(VOCAB_PIDTYPE_TORQUE, jointTrqRef.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint torque references was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointTrqRef, "torque_reference");
        }
    }
    // Read amplifier
    if (settings.logAmplifierQuantities || settings.logAllQuantities) {
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
            bufferManager.push_back(jointPWM, "pwm");
        }

        ok = remappedControlBoardInterfaces.amp->getCurrents(jointCurr.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : current was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointCurr, "current");
        }
    }
    // Read torque
    if (settings.logTorqueQuantities || settings.logAllQuantities) {
        ok = remappedControlBoardInterfaces.itrq->getTorques(jointTrq.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : torque was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointTrq, "torque");
        }
    }

    // Read motor
    if (settings.logMotorQuantities || settings.logAllQuantities) {
        ok = remappedControlBoardInterfaces.imotenc->getMotorEncoders(motorEnc.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : motor encoder was not readed correctly";
        }
        else
        {
            bufferManager.push_back(motorEnc, "motor_encoder");
        }

        ok = remappedControlBoardInterfaces.imotenc->getMotorEncoderSpeeds(motorVel.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : motor velocity was not readed correctly";
        }
        else
        {
            bufferManager.push_back(motorVel, "motor_velocity");
        }

        ok = remappedControlBoardInterfaces.imotenc->getMotorEncoderAccelerations(motorAcc.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : motor acceleration was not readed correctly";
        }
        else
        {
            bufferManager.push_back(motorAcc, "motor_acceleration");
        }
    }

    // Read modes
    if (settings.logModesQuantities || settings.logAllQuantities) {
        ok = remappedControlBoardInterfaces.cmod->getControlModes(controlModes.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : control modes wa not readed correctly";
        }
        else
        {
            bufferManager_modes.push_back(controlModes, "control_modes");
        }

        for (int i = 0; i < interactionModes.size(); i++)
        {
            yarp::dev::InteractionModeEnum tmp;
            ok &= remappedControlBoardInterfaces.imod->getInteractionMode(i, &tmp);
            if (!ok) {
                break;
            }
            if (tmp == VOCAB_IM_STIFF)          interactionModes[i] = 0;
            else if (tmp == VOCAB_IM_COMPLIANT) interactionModes[i] = 1;
            else                                interactionModes[i] = -1;
        }
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : interaction mode was not readed correctly";
        }
        else
        {
            bufferManager_modes.push_back(interactionModes, "interaction_mode");
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

void TelemetryDeviceDumper::run() {
    if (correctlyConfigured) {
        readSensors();
    }
    return;
}

