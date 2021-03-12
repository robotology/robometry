/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef YARP_TELEMETRY_BUFFER_MANAGER_H
#define YARP_TELEMETRY_BUFFER_MANAGER_H

#include <yarp/telemetry/Buffer.h>
#include <yarp/telemetry/BufferConfig.h>

#include <matioCpp/matioCpp.h>

#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>


namespace yarp::telemetry {

/**
 * @brief Class that manages the buffers associated to the channels of the telemetry.
 * Each BufferManager can handle one type of data, the number of samples is defined in the configuration and
 * it is the same for every channel.
 * On the other hand the data inside the channels can have different dimensionality(e.g. 1x1, 2x3 etc).
 * It contains utilities for saving the data of the channels in mat files, and to save/read the configuration
 * to/from a json file.
 *
 */
template<class T>
class BufferManager {

public:
    /**
     * @brief Construct an empty BufferManager object.
     * For being used it has to be configured afterwards.
     *
     */
    BufferManager() = default;

    /**
     * @brief Construct a new BufferManager object, configuring it via
     * the yarp::telemetry::BufferConfig.
     *
     * @param[in] _bufferConfig The struct containing the configuration for the BufferManager.
     */
    BufferManager(const BufferConfig& _bufferConfig) {
        bool ok = configure(_bufferConfig);
        assert(ok);
    }

    /**
     * @brief Destroy the BufferManager object.
     * If auto_save is enabled, it saves to file the remaining data in the buffer.
     *
     */
    ~BufferManager() {
        m_should_stop_thread = true;
        if (m_save_thread.joinable()) {
            m_save_thread.join();
        }
        if (m_bufferConfig.auto_save) {
            saveToFile();
        }
    }

    /**
     * @brief Enable the save thread with _save_period seconds of period.
     * If the thread has been started yet in the configuration through
     * BufferConfing, it skips it.
     *
     * @param[in] _save_period The period in seconds of the save thread.
     * @return true on success, false otherwise.
     */
    bool enablePeriodicSave(double _save_period) {
        // If it is not joinable means it is not running
        if (!m_save_thread.joinable()) {
            m_bufferConfig.save_periodically = true;
            m_bufferConfig.save_period = _save_period;
            m_save_thread = std::thread(&BufferManager::periodicSave, this);
            return true;
        }
        return false;
    }

    /**
     * @brief Configure the BufferManager through a BufferConfig object.
     *
     * @param[in] _bufferConfig The struct containing the configuration parameters.
     * @return true on success, false otherwise.
     */
    bool configure(const BufferConfig& _bufferConfig) {
        bool ok{ true };
        bool shouldResize = _bufferConfig.n_samples != m_bufferConfig.n_samples;
        m_bufferConfig = _bufferConfig;
        if (!_bufferConfig.channels.empty()) {
            ok = ok && addChannels(_bufferConfig.channels);
        }
        if (shouldResize) {
            resize(_bufferConfig.n_samples);
        }
        if (ok && _bufferConfig.save_periodically) {
            ok = ok && enablePeriodicSave(_bufferConfig.save_period);
        }
        populateDescriptionCellArray();
        // TODO ROLL BACK IN CASE OF FAILURE
        return ok;
    }

    /**
     * @brief Get the BufferConfig object representing the actual configuration.
     *
     * @return The BufferConfig object.
     */
    BufferConfig getBufferConfig() const {
        return m_bufferConfig;
    }

    /**
     * @brief Set the file name that will be created by the BufferManager.
     *
     * @param[in] filename The file name to be set.
     */
    void setFileName(const std::string& filename) {
        m_bufferConfig.filename = filename;
        return;
    }

    /**
     * @brief Set the path where the files will be saved.
     *
     * @param[in] path The path to be set.
     */
    void setDefaultPath(const std::string& path) {
        m_bufferConfig.path = path;
        return;
    }

    /**
     * @brief Set the description list that will be saved in all the files.
     *
     * @param[in] description The description to be set.
     */
    void setDescriptionList(const std::vector<std::string>& description_list) {
        m_bufferConfig.description_list = description_list;
        populateDescriptionCellArray();
        return;
    }

    /**
     * @brief Resize the Buffer/s.
     *
     * @param[in] new_size The new size to be resized to.
     */
    void resize(size_t new_size) {
        for (auto& [var_name, buff] : m_buffer_map) {
            buff.resize(new_size);
        }
        m_bufferConfig.n_samples = new_size;
        return;
    }

    /**
     * @brief Add a channel(variable) to the BufferManager.
     * The channels have to be unique in the BufferManager.
     *
     * @param[in] channel Pair representing the channel to be added.
     * @return true on success, false otherwise.
     */
    bool addChannel(const ChannelInfo& channel) {
        // Probably one day we will have just one map
        auto ret_buff = m_buffer_map.insert(std::pair<std::string, yarp::telemetry::Buffer<T>>(channel.first, Buffer<T>(m_bufferConfig.n_samples)));
        auto ret_dim =  m_dimensions_map.insert(std::pair<std::string, yarp::telemetry::dimensions_t>(channel.first, channel.second));
        m_bufferConfig.channels.push_back(channel);
        return ret_buff.second && ret_dim.second;
    }

    /**
     * @brief Add a list of channels(variables) to the BufferManager.
     * The channels have to be unique in the BufferManager.
     *
     * @param[in] channels List of pair representing the channels to be added.
     * @return true on success, false otherwise.
     */
    bool addChannels(const std::vector<ChannelInfo>& channels) {
        if (channels.empty()) {
            return false;
        }
        bool ret{ true };
        for (const auto& c : channels) {
            ret = ret && addChannel(c);
        }
        return ret;
    }

    /**
     * @brief Push a new element in the var_name channel.
     * The var_name channels must exist, otherwise an exception is thrown.
     *
     * @param[in] elem The element to be pushed(via copy) in the channel.
     * @param[in] var_name The name of the channel.
     */
    inline void push_back(const std::vector<T>& elem, const std::string& var_name)
    {
        assert(elem.size() == m_dimensions_map.at(var_name)[0] * m_dimensions_map.at(var_name)[1]);
        assert(m_nowFunction != nullptr);
        m_buffer_map.at(var_name).push_back(Record<T>(m_nowFunction(), elem));
    }

    /**
     * @brief Push a new element in the var_name channel.
     * The var_name channels must exist, otherwise an exception is thrown.
     *
     * @param[in] elem The element to be pushed(via move) in the channel.
     * @param[in] var_name The name of the channel.
     */
    inline void push_back(std::vector<T>&& elem, const std::string& var_name)
    {
        assert(elem.size() == m_dimensions_map.at(var_name)[0] * m_dimensions_map.at(var_name)[1]);
        assert(m_nowFunction != nullptr);
        m_buffer_map.at(var_name).push_back(Record<T>(m_nowFunction(), std::move(elem)));
    }

    /**
     * @brief Save the content of all the channels into a file.
     * If flush_all is set to false, it saves only the content of the channels that
     * have a number of samples greater than the yarp::telemetry::BufferConfig::data_threshold.
     * If yarp::telemetry::BufferConfig::data_threshold is greater than yarp::telemetry::BufferConfig::n_samples
     * this check is skipped.
     *
     * @param[in] flush_all Flag for forcing the save of whatever is contained in the channels.
     * @return true on success, false otherwise.
     */
    bool saveToFile(bool flush_all=true) {

        // now we initialize the proto-timeseries structure
        std::vector<matioCpp::Variable> signalsVect, descrListVect;
        // and the matioCpp struct for these signals
        std::scoped_lock<std::mutex> lock{ m_mutex };
        // Add the description
        if (m_description_cell_array.isValid()) {
            signalsVect.emplace_back(m_description_cell_array);
        }
        // we have to force the flush.
        flush_all = flush_all || (m_bufferConfig.data_threshold > m_bufferConfig.n_samples);
        for (auto& [var_name, buff] : m_buffer_map) {
            if (buff.empty()) {
                std::cout << var_name << " does not contain data, skipping" << std::endl;
                continue;
            }

            if (!flush_all && buff.size() < m_bufferConfig.data_threshold) {
                std::cout << var_name << " does not contain enought data, skipping" << std::endl;
                continue;
            }

            std::vector<T> linear_matrix;
            std::vector<double> timestamp_vector;

            // the number of timesteps is the size of our collection
            auto num_timesteps = buff.size();


            // we first collapse the matrix of data into a single vector, in preparation for matioCpp convertion
            // TODO put mutexes here....
            for (auto& _cell : buff)
            {
                for (auto& _el : _cell.m_datum)
                {
                    linear_matrix.push_back(_el);
                }
                timestamp_vector.push_back(_cell.m_ts);
            }
            buff.clear();

            // now we start the matioCpp convertion process

            // first create timestamps vector
            matioCpp::Vector<double> timestamps("timestamps");
            timestamps = timestamp_vector;

            // and the structures for the actual data too
            std::vector<matioCpp::Variable> test_data;

            // now we create the vector for the dimensions
            // The first two dimensions are the r and c of the sample, the number of sample has to be the last dimension.
            std::vector<int> dimensions_data_vect {(int)m_dimensions_map.at(var_name)[0] , (int)m_dimensions_map.at(var_name)[1], (int)num_timesteps};
            matioCpp::Vector<int> dimensions_data("dimensions");
            dimensions_data = dimensions_data_vect;

            // now we populate the matioCpp matrix
            matioCpp::MultiDimensionalArray<T> out("data", {m_dimensions_map.at(var_name)[0] , m_dimensions_map.at(var_name)[1], (size_t)num_timesteps }, linear_matrix.data());
            test_data.emplace_back(out); // Data

            test_data.emplace_back(dimensions_data); // dimensions vector

            test_data.emplace_back(matioCpp::String("name", var_name)); // name of the signal
            test_data.emplace_back(timestamps);

            // we store it as a matioCpp struct
            matioCpp::Struct data_struct(var_name, test_data);

            // now we create the vector that stores different signals (in case we had more than one)
            signalsVect.emplace_back(data_struct);


        }
        if (signalsVect.empty()) {
            return false;
        }
        // This means that no variables are logged, we have only the description_list
        else if (signalsVect.size() == 1 && m_description_cell_array.isValid()) {
            return false;
        }
        matioCpp::Struct timeSeries(m_bufferConfig.filename, signalsVect);
        // and finally we write the file
        // since we might save several files, we need to index them
        std::string new_file = m_bufferConfig.filename + "_" + std::to_string(m_nowFunction()) + ".mat";
        if (!m_bufferConfig.path.empty()) {
            new_file = m_bufferConfig.path + new_file;
        }
        matioCpp::File file = matioCpp::File::Create(new_file);
        return file.write(timeSeries);
    }

    /**
     * @brief Set the now function, by default is std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count().
     * @param[in] now The now function
     * @return true on success, false otherwise.
     */
    bool setNowFunction(std::function<double(void)> now)
    {
        if (now == nullptr) {
            std::cout << "Not valid clock function." << std::endl;
            return false;
        }

        m_nowFunction = now;
        return true;
    }

private:
    static double DefaultClock() {
        return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
    void periodicSave()
    {
        while (!m_should_stop_thread)
        {
            auto next_step = std::chrono::steady_clock::now() + std::chrono::milliseconds(static_cast<uint32_t>(1000*m_bufferConfig.save_period));

            if (!m_buffer_map.empty()) // if there are channels
            {
                saveToFile(false);
            }
            if (std::chrono::steady_clock::now() < next_step)
            {
                std::this_thread::sleep_until(next_step);
            }
        }
    }

    /**
    * This is an helper function that will be disappear the day matio-cpp
    * will support the std::vector<std::string>
    */
    void populateDescriptionCellArray() {
        if (m_bufferConfig.description_list.empty())
            return;
        std::vector<matioCpp::Variable> descrListVect;
        for (const auto& str : m_bufferConfig.description_list) {
            descrListVect.emplace_back(matioCpp::String("useless_name",str));
        }
        matioCpp::CellArray description_list("description_list", { m_bufferConfig.description_list.size(), 1 }, descrListVect);
        m_description_cell_array = description_list;
    }

    BufferConfig m_bufferConfig;
    std::atomic<bool> m_should_stop_thread{ false };
    std::mutex m_mutex;
    std::unordered_map<std::string, Buffer<T>> m_buffer_map;
    std::unordered_map<std::string, dimensions_t> m_dimensions_map;
    std::function<double(void)> m_nowFunction{DefaultClock};
    std::thread m_save_thread;
    matioCpp::CellArray m_description_cell_array;
};

} // yarp::telemetry

#endif
