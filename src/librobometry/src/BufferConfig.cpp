/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <nlohmann/json.hpp>
#include <robometry/BufferConfig.h>
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

namespace robometry {

    ChannelInfo::ChannelInfo(const std::string& name,
                             const dimensions_t& dimensions,
                             const elements_names_t& elements_names,
                             const units_of_measure_t& units_of_measure) : name(name),
                                                                   dimensions(dimensions),
                                                                   elements_names(elements_names),
                                                                   units_of_measure(units_of_measure)
    {
        const unsigned int elements = std::accumulate(dimensions.begin(),
                                                      dimensions.end(),
                                                      1,
                                                      std::multiplies<>());
        if(elements_names.empty()){
            for (unsigned int i = 0; i < elements; i++) {
                this->elements_names.push_back("element_" + std::to_string(i));
            }
        }
        else {
            if(elements != elements_names.size()) {
                std::cout << "[ChannelInfo::ChannelInfo] The size of the vector elements_names is "
                        << "different from the expected one. Expected: " << elements
                        << "Passed: " << elements_names.size() << std::endl;
            }
        }

        if(!units_of_measure.empty()) {
            // If just one string is specified, let assume that all the data has
            // the same unit of measure
            if (units_of_measure.size()==1) {
                this->units_of_measure.resize(elements, units_of_measure[0]);
            }
            if (elements != units_of_measure.size()) {
                std::cout << "[ChannelInfo::ChannelInfo] The size of the vector units_of_measure is "
                        << "different from the expected one. Expected: " << elements
                        << "Passed: " << units_of_measure.size() << std::endl;
            }
        }

    }

    void to_json(nlohmann::json& j, const ChannelInfo& info)
    {
        j = nlohmann::json{{"name", info.name},
                           {"dimensions", info.dimensions},
                           {"elements_names", info.elements_names},
                           {"units_of_measure", info.units_of_measure}};
    }

    void from_json(const nlohmann::json& j, ChannelInfo& info)
    {
        j.at("name").get_to(info.name);
        j.at("dimensions").get_to(info.dimensions);
        j.at("elements_names").get_to(info.elements_names);
        j.at("units_of_measure").get_to(info.units_of_measure);
    }

    // This expects that the name of the json keyword is the same of the relative variable
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BufferConfig, yarp_robot_name, description_list, path, filename, n_samples, save_period, data_threshold, auto_save, save_periodically, channels, enable_compression, file_indexing, mat_file_version)
}
bool bufferConfigFromJson(robometry::BufferConfig& bufferConfig, const std::string& config_filename) {
    // read a JSON file
    std::ifstream input_stream(config_filename);
    if (!input_stream.is_open()) {
        std::cout << "Failed to open " << config_filename << std::endl;
        return false;
    }
    nlohmann::json jason_file;
    input_stream >> jason_file;
    bufferConfig = jason_file.get<robometry::BufferConfig>();
    input_stream.close();
    return true;
}

bool bufferConfigToJson(const robometry::BufferConfig& bufferConfig, const std::string& config_filename) {
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
