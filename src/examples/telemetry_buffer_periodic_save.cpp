/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */


#include <yarp/os/Time.h>
#include <yarp/os/Network.h>
#include <yarp/telemetry/experimental/BufferManager.h>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <vector>

using namespace std;
using namespace yarp::os;

constexpr size_t n_samples{20};
constexpr size_t threshold{10};
constexpr double check_period{1.0};



int main()
{
    Network yarp;

    yarp::telemetry::experimental::BufferConfig bufferConfig;

    // we configure our API to use our periodic saving option
    bufferConfig.n_samples = n_samples;
    bufferConfig.save_period = check_period;
    bufferConfig.data_threshold = threshold;
    bufferConfig.save_periodically = true;

    yarp::telemetry::experimental::BufferManager<int32_t> bm(bufferConfig);

    std::cout << "First example: " << std::endl;

    bm.setFileName("buffer_manager_test");
    yarp::telemetry::experimental::ChannelInfo var_one{ "one", {1,1} };
    yarp::telemetry::experimental::ChannelInfo var_two{ "two", {1,1} };

    auto ok = bm.addChannel(var_one);
    ok = ok && bm.addChannel(var_two);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }
    std::cout << "Starting loop" << std::endl;
    for (int i = 0; i < 40; i++) {
        std::vector tmp = { i };
        bm.push_back(tmp, "one");
        yarp::os::Time::delay(0.01);
        tmp[0]++;
        bm.push_back(tmp, "two");
    }

    // now we use also the auto_saving option
    bufferConfig.auto_save = true;

    std::cout << "Second example: " << std::endl;

    yarp::telemetry::experimental::BufferManager<int32_t> bm_m(bufferConfig);
    bm_m.setFileName("buffer_manager_test_matrix");
    std::vector<yarp::telemetry::experimental::ChannelInfo> vars{ { "one",{2,3} },
                                   { "two",{3,2} } };

    ok = bm_m.addChannels(vars);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }

    for (int i = 0; i < 40; i++) {
        std::vector tmp1 = { i + 1, i + 2, i + 3, i + 4, i + 5, i + 6 };
        bm_m.push_back(tmp1, "one");
        yarp::os::Time::delay(0.01);
        std::vector tmp2 = { i * 1, i * 2, i * 3, i * 4, i * 5, i * 6 };
        bm_m.push_back(tmp2, "two");
    }

    std::cout << "Third example: " << std::endl;

    bufferConfig.channels = { {"one",{4,1}}, {"two",{4,1}} };
    bufferConfig.filename = "buffer_manager_test_vector";

    yarp::telemetry::experimental::BufferManager<double> bm_v(bufferConfig);

    for (int i = 0; i < 40; i++) {
        std::vector tmp1 = { i+1.0, i+2.0, i+3.0, i+4.0  };
        bm_v.push_back(tmp1, "one");
        yarp::os::Time::delay(0.01);
        std::vector tmp2 = { (double)i, i*2.0, i*3.0, i*4.0 };
        bm_v.push_back(tmp2, "two");
    }

    return 0;
 }
