/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <nlohmann/json.hpp>
#include <yarp/telemetry/experimental/BufferConfig.h>
#include <fstream>
#include <iostream>

namespace matioCpp {
    NLOHMANN_JSON_SERIALIZE_ENUM( FileVersion, {
            {FileVersion::Undefined, "undefined"},
            {FileVersion::MAT4, "v4"},
            {FileVersion::MAT5, "v5"},
            {FileVersion::MAT7_3, "v7_3"},
            {FileVersion::Default, "default"},
        })
}

namespace yarp::telemetry::experimental {
    // This expects that the name of the json keyword is the same of the relative variable
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BufferConfig, description_list, path, filename, n_samples, save_period, data_threshold, auto_save, save_periodically, channels, enable_compression, file_indexing, mat_file_version)
}
bool bufferConfigFromJson(yarp::telemetry::experimental::BufferConfig& bufferConfig, const std::string& config_filename) {
    // read a JSON file
    std::ifstream input_stream(config_filename);
    if (!input_stream.is_open()) {
        std::cout << "Failed to open " << config_filename << std::endl;
        return false;
    }
    nlohmann::json jason_file;
    input_stream >> jason_file;
    bufferConfig = jason_file.get<yarp::telemetry::experimental::BufferConfig>();
    input_stream.close();
    return true;
}

bool bufferConfigToJson(const yarp::telemetry::experimental::BufferConfig& bufferConfig, const std::string& config_filename) {
    // write to a JSON file
    std::ofstream out_stream(config_filename);
    if (!out_stream.is_open()) {
        std::cout << "Failed to open " << config_filename << std::endl;
        return false;
    }
    nlohmann::json j = bufferConfig;
    out_stream << j;
    out_stream.close();
    return true;
}
