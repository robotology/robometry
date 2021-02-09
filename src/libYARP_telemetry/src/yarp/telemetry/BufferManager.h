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

template<class T>
class BufferManager {

public:
    BufferManager() = default;

    BufferManager(const BufferConfig& _bufferConfig) {
        bool ok = configure(_bufferConfig);
        assert(ok);
    }

    ~BufferManager() {
        m_should_stop_thread = true;
        if (m_bufferConfig.auto_save) {
            saveToFile();
        }
    }
    // This function is used for manual toggling the periodic save, then
    // if the thread has been started yet in the configuration throught
    // BufferConfing, it skip
    bool enablePeriodicSave(double _save_period) {
        if (!m_thread_running) {
            m_bufferConfig.save_periodically = true;
            m_bufferConfig.save_period = _save_period;
            std::thread save_thread(&BufferManager::periodicSave, this);
            save_thread.detach();
            return true;
        }
        return false;
    }

    bool configure(const BufferConfig& _bufferConfig) {
        bool ok{ true };
        m_bufferConfig = _bufferConfig;
        if (!_bufferConfig.channels.empty()) {
            ok = ok && addChannels(_bufferConfig.channels);
        }
        if (ok && _bufferConfig.save_periodically) {
            ok = ok && enablePeriodicSave(_bufferConfig.save_period);
        }
        // TODO ROLL BACK IN CASE OF FAILURE
        return ok;
    }

    BufferConfig getBufferConfig() const {
        return m_bufferConfig;
    }

    void setFileName(const std::string& filename) {
        m_bufferConfig.filename = filename;
        return;
    }

    void resize(size_t new_size) {
        for (auto& [var_name, buff] : m_buffer_map) {
            buff.resize(new_size);
        }
        m_bufferConfig.n_samples = new_size;
        return;
    }

    bool addChannel(const ChannelInfo& channel) {
        // Probably one day we will have just one map
        auto ret_buff = m_buffer_map.insert(std::pair<std::string, yarp::telemetry::Buffer<T>>(channel.first, Buffer<T>(m_bufferConfig.n_samples)));
        auto ret_dim =  m_dimensions_map.insert(std::pair<std::string, yarp::telemetry::dimensions_t>(channel.first, channel.second));
        m_bufferConfig.channels.push_back(channel);
        return ret_buff.second && ret_dim.second;
    }

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

    inline void push_back(const std::vector<T>& elem, const std::string& var_name)
    {
        assert(elem.size() == m_dimensions_map.at(var_name)[0] * m_dimensions_map.at(var_name)[1]);
        assert(m_nowFunction != nullptr);
        m_buffer_map.at(var_name).push_back(Record<T>(m_nowFunction(), elem));
    }


    inline void push_back(std::vector<T>&& elem, const std::string& var_name)
    {
        assert(elem.size() == m_dimensions_map.at(var_name)[0] * m_dimensions_map.at(var_name)[1]);
        assert(m_nowFunction != nullptr);
        m_buffer_map.at(var_name).push_back(Record<T>(m_nowFunction(), std::move(elem)));
    }

    bool saveToFile(bool flush_all=true) {

        // now we initialize the proto-timeseries structure
        std::vector<matioCpp::Variable> signalsVect;
        // and the matioCpp struct for these signals
        std::scoped_lock<std::mutex> lock{ m_mutex };
        // In case of the misconfiguration where the threshold is less than the capacity of buffers
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
            std::cout << "No available data to be saved" << std::endl;
            return false;
        }
        matioCpp::Struct timeSeries(m_bufferConfig.filename, signalsVect);
        // and finally we write the file
        // since we might save several files, we need to index them
        std::string new_file = m_bufferConfig.filename + "_" + std::to_string(file_index) + ".mat";
        file_index++;
        matioCpp::File file = matioCpp::File::Create(new_file);
        return file.write(timeSeries);
    }

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
            m_thread_running = true;
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
        m_thread_running = false;
    }

    BufferConfig m_bufferConfig;
    std::atomic<bool> m_should_stop_thread{ false }, m_thread_running{ false };
    std::mutex m_mutex;
    int file_index{ 0 };
    std::unordered_map<std::string, Buffer<T>> m_buffer_map;
    std::unordered_map<std::string, dimensions_t> m_dimensions_map;
    std::function<double(void)> m_nowFunction{DefaultClock};
};

} // yarp::telemetry

#endif
