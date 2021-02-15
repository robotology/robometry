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
/**
 * @brief Pair representing a channel(variable) in terms of
 * name and dimensions.
 */
using ChannelInfo = std::pair< std::string, dimensions_t >;
/**
 * @brief Struct containing the parameters for configuring a yarp::telemetry::BufferManager.
 *
 */
struct YARP_telemetry_API BufferConfig {
    std::string filename{ "" };/**< the file name, to it will be appended the suffix "_<timestamp>.mat". */
    size_t n_samples{ 0 };/**< the max number of samples contained in the buffer/s */
    double save_period{ 0.010 };/**< the period in sec of the save thread */
    size_t data_threshold{ 0 };/**< the save thread saves to a file if there are at least data_threshold samples */
    bool auto_save{ false };/**< the flag for enabling the save in the destructor of the yarp::telemetry::BufferManager */
    bool save_periodically{ false };/**< the flag for enabling the periodic save thread. */
    std::vector<ChannelInfo> channels;/**< the list of pairs representing the channels(variables) */
};

} // yarp::telemetry

/**
 * @brief Populate the yarp::telemetry::BufferConfig struct reading from a json file.
 *
 * @param[out] bufferConfig The struct to be filled in.
 * @param[in] config_filename The name of the json file.
 * @return true on success, false otherwise.
 */
bool YARP_telemetry_API bufferConfigFromJson(yarp::telemetry::BufferConfig& bufferConfig, const std::string& config_filename);

/**
 * @brief Save on a json file the content of a yarp::telemetry::BufferConfig struct.
 *
 * @param[in] bufferConfig The struct to be saved to file.
 * @param[in] config_filename The name of the json file to be saved.
 * @return true on success, false otherwise.
 */
bool YARP_telemetry_API bufferConfigToJson(const yarp::telemetry::BufferConfig& bufferConfig, const std::string& config_filename);

#endif
