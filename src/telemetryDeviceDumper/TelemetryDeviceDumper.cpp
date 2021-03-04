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

using namespace yarp::telemetry;
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
        settings.logJointVelocity = prop.find(useJointVelocityOptionName.c_str()).asBool();
    }

    std::string useJointAccelerationOptionName = "logJointAcceleration";
    if (prop.check(useJointAccelerationOptionName.c_str())) {
        settings.logJointAcceleration = prop.find(useJointAccelerationOptionName.c_str()).asBool();
    }

    std::string useRadians = "useRadians";
    if (prop.check(useRadians.c_str())) {
        settings.useRadians = prop.find(useRadians.c_str()).asBool();
    }

    std::string experimentName = "experimentName";
    if (prop.check(experimentName.c_str()) && prop.find(experimentName.c_str()).isString()) {
        settings.experimentName = prop.find(experimentName.c_str()).asString();
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

    addVectorOfStringToProperty(propRemapper, "axesNames", jointNames);

    ok = remappedControlBoard.open(propRemapper);

    if (!ok)
    {
        return ok;
    }

    // View relevant interfaces for the remappedControlBoard
    ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.encs);
    ok = ok && remappedControlBoard.view(remappedControlBoardInterfaces.multwrap);

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
}

bool TelemetryDeviceDumper::configBufferManager(yarp::os::Searchable& conf) {
    // This is the buffer manager configuration
    yarp::telemetry::BufferConfig bufferConfig;
    bool ok{ true };
    ok = ok && bufferManager.addChannel({ "encoders", {1, jointPos.size()} });
    if (ok && settings.logJointVelocity) {
        ok = ok && bufferManager.addChannel({ "velocity", {1, jointVel.size()} });
    }

    if (ok && settings.logJointAcceleration) {
        ok = ok && bufferManager.addChannel({ "acceleration", {1, jointAcc.size()} });
    }

    bufferConfig.filename = settings.experimentName;
    
    // TODO add joint names via description field to be added to the API

    // TODO set path where to log data, it has to be done in the API

    // TODO for now it is just hardcoded
    bufferConfig.n_samples = 1000;
    bufferConfig.save_period = 1.0;
    bufferConfig.data_threshold = 300;
    bufferConfig.save_periodically = true;

    ok = ok && bufferManager.configure(bufferConfig);

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
    return  this->remappedControlBoardInterfaces.multwrap->detachAll();;
}

bool TelemetryDeviceDumper::close()
{
    correctlyConfigured = false;
    remappedControlBoard.close();
    // Flush all the remaining data.
    bufferManager.saveToFile();

    return true;
}


void TelemetryDeviceDumper::readSensors()
{
    // Read encoders
    sensorsReadCorrectly = remappedControlBoardInterfaces.encs->getEncoders(jointPos.data());


    bool ok;

    if (!sensorsReadCorrectly)
    {
        yWarning() << "telemetryDeviceDumper warning : joint positions was not readed correctly";
    }
    else
    {
        bufferManager.push_back(jointPos, "encoders");
    }

    // At the moment we are assuming that all joints are revolute

    if (settings.logJointVelocity)
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

    if (settings.logJointAcceleration)
    {
        ok = remappedControlBoardInterfaces.encs->getEncoderAccelerations(jointAcc.data());
        sensorsReadCorrectly = sensorsReadCorrectly && ok;
        if (!ok)
        {
            yWarning() << "telemetryDeviceDumper warning : joint accelerations was not readed correctly";
        }
        else
        {
            bufferManager.push_back(jointPos, "acceleration");
        }


    }
    if (settings.useRadians) {
        // TODO check if it is safe to call transform on empty vectors.
        convertVectorFromDegreesToRadians(jointPos);
        convertVectorFromDegreesToRadians(jointVel);
        convertVectorFromDegreesToRadians(jointAcc);
    }

}

void TelemetryDeviceDumper::run() {
    if (correctlyConfigured) {
        readSensors();

    }
    return;
}

