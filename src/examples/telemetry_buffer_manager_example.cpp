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

constexpr size_t n_samples{3};


int main()
{
    Network yarp;
    auto now = yarp::os::Time::now;

    yarp::telemetry::BufferManager<int32_t> bm(n_samples);
    bm.setFileName("buffer_manager_test.mat");
    auto ok = bm.setNowFunction(now);
    if (!ok) {
        std::cout << "Problem setting the clock...."<<std::endl;
        return 1;
    }

    yarp::telemetry::ChannelInfo var_one{ "one", {1,1} };
    yarp::telemetry::ChannelInfo var_two{ "two", {1,1} };

    ok = bm.addChannel(var_one);
    ok = ok && bm.addChannel(var_two);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        bm.push_back({ i }, "one");
        yarp::os::Time::delay(0.2);
        bm.push_back({ i + 1 }, "two");
    }

    if (bm.saveToFile())
        std::cout << "File saved correctly!" << std::endl;
    else
        std::cout << "Something went wrong..." << std::endl;

    yarp::telemetry::BufferManager<int32_t> bm_m(n_samples, true);
    bm_m.setFileName("buffer_manager_test_matrix.mat");
    ok = bm_m.setNowFunction(now);
    if (!ok) {
        std::cout << "Problem setting the clock...."<<std::endl;
        return 1;
    }


    std::vector<yarp::telemetry::ChannelInfo> vars{ { "one",{2,3} },
                                   { "two",{3,2} } };

    ok = bm_m.addChannels(vars);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        bm_m.push_back({ i + 1, i + 2, i + 3, i + 4, i + 5, i + 6 }, "one");
        yarp::os::Time::delay(0.2);
        bm_m.push_back({ i * 1, i * 2, i * 3, i * 4, i * 5, i * 6 }, "two");
    }

    yarp::telemetry::BufferManager<double> bm_v("buffer_manager_test_vector.mat",
                                               { {"one",{4,1}},
                                                 {"two",{4,1}} }, n_samples, true);
    ok = bm_v.setNowFunction(now);
    if (!ok) {
        std::cout << "Problem setting the clock...."<<std::endl;
        return 1;
    }


    for (int i = 0; i < 10; i++) {
        bm_v.push_back({ i+1.0, i+2.0, i+3.0, i+4.0  }, "one");
        yarp::os::Time::delay(0.2);
        bm_v.push_back({ (double)i, i*2.0, i*3.0, i*4.0 }, "two");
    }

    return 0;
 }
