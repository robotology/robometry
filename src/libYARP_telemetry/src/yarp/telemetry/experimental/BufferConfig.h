/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef YARP_TELEMETRY_BUFFER_CONFIG_H
#define YARP_TELEMETRY_BUFFER_CONFIG_H

#include <matioCpp/File.h>
#include <matioCpp/ForwardDeclarations.h>
#include <yarp/telemetry/experimental/api.h>
#include <string>
#include <vector>
#include <numeric>

namespace yarp::telemetry::experimental {
using dimensions_t = std::vector<size_t>;
using elements_names_t = std::vector<std::string>;
/**
 * @brief Struct representing a channel(variable) in terms of
 * name and dimensions and names of the each element of a variable.
 */
struct YARP_telemetry_API ChannelInfo {
    std::string name; /**< Name of the channel */
    dimensions_t dimensions; /**< Dimension of the channel */
    elements_names_t elements_names; /**< Vector containing the names of each element of the channel */

    /**
     * @brief Default constructor
     */
    ChannelInfo() = default;

    /**
     * @brief Construct a ChannelInfo from name, dimensions and a vector containing the name of
     * the elements associated to the channel.
     * @param name name of the channel.
     * @param dimensions dimension associated to the channel.
     * @param elements_names Vector containing the names of each element of the channel.
     */
    ChannelInfo(const std::string& name,
                const dimensions_t& dimensions,
                const elements_names_t& elements_names);

    /**
     * @brief Construct a ChannelInfo from name and dimensions.
     * @param name name of the channel.
     * @param dimensions dimension associated to the channel.
     * @note If the constructor is called the elements_names are set as
     * elements_names = [element_0, element_1, ..., element_n], where n is given by the
     * product of the dimensions.
     */
    ChannelInfo(const std::string& name, const dimensions_t& dimensions);
};

/**
 * @brief Struct containing the parameters for configuring a yarp::telemetry::experimental::BufferManager.
 *
 */
struct YARP_telemetry_API BufferConfig {
    std::string yarp_robot_name{""}; /** < The yarp robot name associated to the machine where the logger runs */
    std::vector<std::string> description_list{""}; /** < the description list, e.g. it can contain the axes names that are logged*/
    std::string path{ "" }; /**< the path in which the files will be saved. */
    std::string filename{ "" };/**< the file name, to it will be appended the suffix "_<timestamp>.mat". */
    size_t n_samples{ 0 };/**< the max number of samples contained in the buffer/s */
    double save_period{ 0.010 };/**< the period in sec of the save thread */
    size_t data_threshold{ 0 };/**< the save thread saves to a file if there are at least data_threshold samples */
    bool auto_save{ false };/**< the flag for enabling the save in the destructor of the yarp::telemetry::experimental::BufferManager */
    bool save_periodically{ false };/**< the flag for enabling the periodic save thread. */
    std::vector<ChannelInfo> channels;/**< the list of pairs representing the channels(variables) */
    bool enable_compression{ false }; /**< the flag for enabling the zlib compression */
     /** String representing the indexing mode. If the variable is set to `time_since_epoch`, `BufferManager::m_nowFunction`
      * is used. Othewrise `std::put_time` is used to generate the indexing. https://en.cppreference.com/w/cpp/io/manip/put_time */
    std::string file_indexing{ "time_since_epoch" };
    matioCpp::FileVersion mat_file_version{ matioCpp::FileVersion::Default }; /**< Version of the saved matfile.  */
};

} // yarp::telemetry::experimental

/**
 * @brief Populate the yarp::telemetry::experimental::BufferConfig struct reading from a json file.
 *
 * @param[out] bufferConfig The struct to be filled in.
 * @param[in] config_filename The name of the json file.
 * @return true on success, false otherwise.
 */
bool YARP_telemetry_API bufferConfigFromJson(yarp::telemetry::experimental::BufferConfig& bufferConfig, const std::string& config_filename);

/**
 * @brief Save on a json file the content of a yarp::telemetry::experimental::BufferConfig struct.
 *
 * @param[in] bufferConfig The struct to be saved to file.
 * @param[in] config_filename The name of the json file to be saved.
 * @return true on success, false otherwise.
 */
bool YARP_telemetry_API bufferConfigToJson(const yarp::telemetry::experimental::BufferConfig& bufferConfig, const std::string& config_filename);

#endif
