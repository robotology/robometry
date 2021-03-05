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

namespace yarp::telemetry {

/**
 * @brief FILL DOCUMENTATION
 *
 */
struct RemoteControlGateway {
    std::unique_ptr<yarp::dev::PolyDriver> m_remoteControlBoard{ nullptr };
    yarp::telemetry::BufferManager<double> m_bm;
    yarp::dev::IEncoders* m_iEnc{ nullptr };
    std::vector<double> m_encs_vec, m_encs_speeds_vec, m_encs_acc_vec;

    bool open(const std::string& robot, const std::string& part, const std::string& moduleName = "telemetryDeviceDumper");

    void readAndPush();

    void close();
};
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

private:
    std::unordered_map<std::string,yarp::telemetry::RemoteControlGateway> m_rcg_map;

};
}

#endif // YARP_TELEMETRY_TELEMETRYDEVICEDUMPER_H
