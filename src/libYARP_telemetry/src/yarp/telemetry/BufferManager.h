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
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <assert.h>
#include <functional>
#include <chrono>
#include <matioCpp/matioCpp.h>


namespace yarp::telemetry {

using dimensions_t = std::vector<size_t>;

struct ChannelInfo {
    std::string m_var_name;
    dimensions_t m_dimensions{ 1,1 };
};



template<class T>
class BufferManager {

public:
    BufferManager() = delete;
    explicit BufferManager(size_t n_samples, bool auto_save = false) : m_n_samples(n_samples), m_auto_save(auto_save) {
    }
    BufferManager(const std::string& filename, const std::vector<ChannelInfo>& channels,
                  size_t n_samples, bool auto_save=false) : m_filename(filename), m_auto_save(auto_save), m_n_samples(n_samples){
        assert(!channels.empty());
        assert(!filename.empty());
        assert(addChannels(channels) == true);
	}

    ~BufferManager() {
        if (m_auto_save) {
            saveToFile();
        }
    }

    void setFileName(const std::string& filename) {
        m_filename = filename;
        return;
    }

    bool addChannel(const ChannelInfo& channel) {
        // Probably one day we will have just one map
        auto ret_buff = m_buffer_map.insert(std::pair<std::string, yarp::telemetry::Buffer<T>>(channel.m_var_name, Buffer<T>(m_n_samples)));
        auto ret_dim =  m_dimensions_map.insert(std::pair<std::string, yarp::telemetry::dimensions_t>(channel.m_var_name, channel.m_dimensions));
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

    bool saveToFile() {

        // now we initialize the proto-timeseries structure
        std::vector<matioCpp::Variable> signalsVect;
        // and the matioCpp struct for these signals
        for (auto& [var_name, buff] : m_buffer_map) {
            if (buff.empty())
            {
                std::cout << var_name << " does not contain data, skipping" << std::endl;
                continue;
            }

            std::vector<T> linear_matrix;
            std::vector<double> timestamp_vector;

            // the number of timesteps is the size of our collection
            int num_timesteps = buff.size();


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
            std::vector<int> dimensions_data_vect {(int)m_dimensions_map.at(var_name)[0] , (int)m_dimensions_map.at(var_name)[1], num_timesteps};
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
        auto point_pos = m_filename.find('.');
        matioCpp::Struct timeSeries(std::string(m_filename.begin(), m_filename.begin()+point_pos), signalsVect);
        // and finally we write the file
        matioCpp::File file = matioCpp::File::Create(m_filename);
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

    std::string m_filename;
    bool m_auto_save{false};
    size_t m_n_samples{0};
    std::unordered_map<std::string, Buffer<T>> m_buffer_map;
    std::unordered_map<std::string, dimensions_t> m_dimensions_map;
    std::function<double(void)> m_nowFunction{DefaultClock};
};

} // yarp::telemetry

#endif
