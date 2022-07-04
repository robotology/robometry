/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef ROBOMETRY_BUFFER_CONFIG_H
#define ROBOMETRY_BUFFER_CONFIG_H

#include <matioCpp/File.h>
#include <matioCpp/ForwardDeclarations.h>
#include <string>
#include <vector>
#include <numeric>

namespace robometry {
using dimensions_t = std::vector<size_t>;
using elements_names_t = std::vector<std::string>;
using units_of_measure_t = std::vector<std::string>;
/**
 * @brief Struct representing a channel(variable) in terms of
 * name and dimensions and names of the each element of a variable.
 */
struct ChannelInfo {
    std::string name; /**< Name of the channel */
    dimensions_t dimensions; /**< Dimension of the channel */
    elements_names_t elements_names; /**< Vector containing the names of each element of the channel */
    units_of_measure_t units_of_measure; /**< Units of measure of the channel */
    /**
     * @brief Default constructor
     */
    ChannelInfo() = default;
    /**
     *@brief Construct a ChannelInfo from name, dimensions, a vector containing the names
      of each element of the channel and the unit of measure of the channel.
      @param name name of the channel.
      @param dimensions dimensions of the channel.
      @param elements_names vector containing the names of each element of the channel.
      @param units_of_measure unit of measure of the channel.
     */
    ChannelInfo(const std::string& name,
                const dimensions_t& dimensions,
                const elements_names_t& elements_names=elements_names_t(),
                const units_of_measure_t& units_of_measure=units_of_measure_t());


};

/**
 * @brief Struct containing the parameters for configuring a robometry::BufferManager.
 *
 */
struct BufferConfig {
    std::string yarp_robot_name{""}; /** < The yarp robot name associated to the machine where the logger runs */
    std::vector<std::string> description_list{""}; /** < the description list, e.g. it can contain the axes names that are logged*/
    std::string path{ "" }; /**< the path in which the files will be saved. */
    std::string filename{ "yarp_telemetry_log" };/**< the file name, to it will be appended the suffix "_<timestamp>.mat". */
    size_t n_samples{ 0 };/**< the max number of samples contained in the buffer/s */
    double save_period{ 0.010 };/**< the period in sec of the save thread */
    size_t data_threshold{ 0 };/**< the save thread saves to a file if there are at least data_threshold samples */
    bool auto_save{ false };/**< the flag for enabling the save in the destructor of the robometry::BufferManager */
    bool save_periodically{ false };/**< the flag for enabling the periodic save thread. */
    std::vector<ChannelInfo> channels;/**< the list of pairs representing the channels(variables) */
    bool enable_compression{ false }; /**< the flag for enabling the zlib compression */
     /** String representing the indexing mode. If the variable is set to `time_since_epoch`, `BufferManager::m_nowFunction`
      * is used. Othewrise `std::put_time` is used to generate the indexing. https://en.cppreference.com/w/cpp/io/manip/put_time */
    std::string file_indexing{ "time_since_epoch" };
    matioCpp::FileVersion mat_file_version{ matioCpp::FileVersion::Default }; /**< Version of the saved matfile.  */
};

} // robometry

/**
 * @brief Populate the robometry::BufferConfig struct reading from a json file.
 *
 * @param[out] bufferConfig The struct to be filled in.
 * @param[in] config_filename The name of the json file.
 * @return true on success, false otherwise.
 */
bool bufferConfigFromJson(robometry::BufferConfig& bufferConfig, const std::string& config_filename);

/**
 * @brief Save on a json file the content of a robometry::BufferConfig struct.
 *
 * @param[in] bufferConfig The struct to be saved to file.
 * @param[in] config_filename The name of the json file to be saved.
 * @return true on success, false otherwise.
 */
bool bufferConfigToJson(const robometry::BufferConfig& bufferConfig, const std::string& config_filename);

#endif
