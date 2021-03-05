/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */
#include "TelemetryDeviceDumper.h"
#include <array>

using namespace yarp::telemetry;
using namespace yarp::os;

constexpr double period_thread{ 0.010 };
constexpr std::array<char*, 6> partsArray{ "head","torso","left_arm","right_arm","left_leg","right_leg" };


bool RemoteControlGateway::open(const std::string& robot, const std::string& part, const std::string& moduleName) {
    yarp::os::Property conf{ {"device", yarp::os::Value("remote_controlboard")},
                             {"remote", yarp::os::Value("/" + robot + "/" + part)},
                             {"local",  yarp::os::Value("/" + moduleName + "/" + part + "/remoteControlBoard")} };


    m_remoteControlBoard = std::make_unique<yarp::dev::PolyDriver>();

    bool ok = m_remoteControlBoard->open(conf);
    if (!ok) {
        yError() << "RemoteControlGateway: failed to open the remote control board for part" << part;
        return false;
    }
    ok = ok && m_remoteControlBoard->view(m_iEnc);
    int num_axes{ 0 };
    ok = ok && m_iEnc->getAxes(&num_axes);

    if (ok) {
        m_encs_vec.resize(num_axes);
        m_encs_speeds_vec.resize(num_axes);
        m_encs_acc_vec.resize(num_axes);
        
        // This is the buffer manager configuration
        // For now it is just hard coded.
        yarp::telemetry::BufferConfig bufferConfig;
        bufferConfig.channels = { {"encoders",{1,(size_t)num_axes}},
                                  {"encoder_speeds",{1,(size_t)num_axes}},
                                  {"encoder_accelerations",{1,(size_t)num_axes}} };
        bufferConfig.filename = moduleName + "_" + robot + "_" + part;
        bufferConfig.n_samples = 1000;
        bufferConfig.save_period = 1.0;
        bufferConfig.data_threshold = 300;
        bufferConfig.save_periodically = true;

        ok = ok && m_bm.configure(bufferConfig);
    }

    return ok;
}

void RemoteControlGateway::readAndPush() {
    if (!m_iEnc) {
        return;
    }
    bool ok = m_iEnc->getEncoders(m_encs_vec.data());
    ok = ok && m_iEnc->getEncoderSpeeds(m_encs_speeds_vec.data());
    ok = ok && m_iEnc->getEncoderAccelerations(m_encs_acc_vec.data());
    if (ok) {
        m_bm.push_back(m_encs_vec, "encoders");
        m_bm.push_back(m_encs_speeds_vec, "encoder_speeds");
        m_bm.push_back(m_encs_acc_vec, "encoder_accelerations");
    }
}

void RemoteControlGateway::close() {
    m_remoteControlBoard->close();
}

// For now also the period is harcoded, it can be configured
TelemetryDeviceDumper::TelemetryDeviceDumper() : yarp::os::PeriodicThread(period_thread) {
    // TODO
}

TelemetryDeviceDumper::~TelemetryDeviceDumper() {
    // TODO
}

bool TelemetryDeviceDumper::close() {
    for (auto& r : m_rcg_map) {
        r.second.close();
    }
    PeriodicThread::stop();
    return true;
}

bool TelemetryDeviceDumper::open(yarp::os::Searchable& config) {
    auto robot = config.check("robot", Value("icubSim")).asString();
    auto name = config.check("name", Value("telemetryDeviceDumper")).asString();

    auto partsBot = config.find("parts").asList();
    if (!partsBot || partsBot->isNull())
    {
        yError() << "telemetryDeviceDumper: missing parts parameter, please specify as list";
        return false;
    }

    for (size_t i = 0; i < partsBot->size(); i++) {
        auto partStr = partsBot->get(i).asString();
        if (partsArray.end() == std::find(partsArray.begin(), partsArray.end(), partStr))
        {
            yError() << "telemetryDeviceDumper: the part" << partStr << "is not available";
            return false;
        }
        if (!m_rcg_map[partStr].open(robot, partStr, name))
        {
            yError() << "telemetryDeviceDumper: failed to open one remote control gateway.. closing.";
            return false;
        }
    }

    return PeriodicThread::start();
}

bool TelemetryDeviceDumper::attachAll(const yarp::dev::PolyDriverList& device2attach) {
    // TODO
    return true;
}

bool TelemetryDeviceDumper::detachAll() {
    // TODO
    return true;
}

bool TelemetryDeviceDumper::attach(yarp::dev::PolyDriver* poly) {
    // TODO
    return true;
}

bool TelemetryDeviceDumper::detach() {
    // TODO
    return true;
}

// PeriodicThread
bool TelemetryDeviceDumper::threadInit() {
    // TODO
    return true;
}

void TelemetryDeviceDumper::threadRelease() {
    // TODO
    return;
}

void TelemetryDeviceDumper::run() {
    for (auto& rcg : m_rcg_map) {
        rcg.second.readAndPush();
    }
    return;
}

