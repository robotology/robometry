/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef YARP_TELEMETRY_BUFFER_CONFIG_H
#define YARP_TELEMETRY_BUFFER_CONFIG_H

#include <yarp/telemetry/api.h>
#include <cstring>
#include <vector>

namespace yarp::telemetry {
using dimensions_t = std::vector<size_t>;

using ChannelInfo = std::pair< std::string, dimensions_t >;

struct YARP_telemetry_API BufferConfig {
    std::string filename{ "" };
    size_t n_samples{ 0 };
    double save_period{ 0.010 };
    size_t data_threshold{ 0 };
    bool auto_save{ false };
    bool save_periodically{ false };
    std::vector<ChannelInfo> channels;
};

} // yarp::telemetry

bool YARP_telemetry_API bufferConfigFromJson(yarp::telemetry::BufferConfig& bufferConfig, const std::string& config_filename);

bool YARP_telemetry_API bufferConfigToJson(const yarp::telemetry::BufferConfig& bufferConfig, const std::string& config_filename);

#endif
