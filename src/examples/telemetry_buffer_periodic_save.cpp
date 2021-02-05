/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */


#include <yarp/os/Time.h>
#include <yarp/os/Network.h>
#include <yarp/telemetry/BufferManager.h>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <vector>

using namespace std;
using namespace yarp::os;

constexpr size_t n_samples{20};
constexpr size_t threshold{10};
constexpr double check_period{100.0};



int main()
{
    Network yarp;
    
    yarp::telemetry::BufferConfig bufferConfig;

    // we configure our API to use our periodic saving option 
    bufferConfig.n_samples = n_samples;
    bufferConfig.check_period = check_period;
    bufferConfig.threshold = threshold;
    bufferConfig.save_periodically = true;

    yarp::telemetry::BufferManager<int32_t> bm(bufferConfig);

    std::cout << "First example: " << std::endl;

    bm.setFileName("buffer_manager_test");
    yarp::telemetry::ChannelInfo var_one{ "one", {1,1} };
    yarp::telemetry::ChannelInfo var_two{ "two", {1,1} };

    auto ok = bm.addChannel(var_one);
    ok = ok && bm.addChannel(var_two);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }

    for (int i = 0; i < 40; i++) {
        bm.push_back({ i }, "one");
        yarp::os::Time::delay(0.2);
        bm.push_back({ i + 1 }, "two");
    }

    std::cout << "Second example: " << std::endl;

    // now we use also the auto_saving option
    bufferConfig.m_auto_save = true;

    yarp::telemetry::BufferManager<int32_t> bm_m(bufferConfig);
    bm_m.setFileName("buffer_manager_test_matrix");
    std::vector<yarp::telemetry::ChannelInfo> vars{ { "one",{2,3} },
                                   { "two",{3,2} } };

    ok = bm_m.addChannels(vars);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }

    for (int i = 0; i < 40; i++) {
        bm_m.push_back({ i + 1, i + 2, i + 3, i + 4, i + 5, i + 6 }, "one");
        yarp::os::Time::delay(0.2);
        bm_m.push_back({ i * 1, i * 2, i * 3, i * 4, i * 5, i * 6 }, "two");
    }

    std::cout << "Third example: " << std::endl;

    yarp::telemetry::BufferManager<double> bm_v("buffer_manager_test_vector",
                                               { {"one",{4,1}},
                                                 {"two",{4,1}} }, bufferConfig);
    // bm_v.periodicSave();
    for (int i = 0; i < 40; i++) {
        bm_v.push_back({ i+1.0, i+2.0, i+3.0, i+4.0  }, "one");
        yarp::os::Time::delay(0.2);
        bm_v.push_back({ (double)i, i*2.0, i*3.0, i*4.0 }, "two");
    }

    return 0;
 }
