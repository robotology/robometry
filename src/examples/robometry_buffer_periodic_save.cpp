/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */


#include <robometry/BufferManager.h>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <vector>

using namespace std;

constexpr size_t n_samples{20};
constexpr size_t threshold{10};
constexpr double check_period{1.0};



int main()
{

    robometry::BufferConfig bufferConfig;

    // we configure our API to use our periodic saving option
    bufferConfig.n_samples = n_samples;
    bufferConfig.save_period = check_period;
    bufferConfig.data_threshold = threshold;
    bufferConfig.save_periodically = true;

    robometry::BufferManager bm(bufferConfig);

    std::cout << "First example: " << std::endl;

    bm.setFileName("buffer_manager_test");
    robometry::ChannelInfo var_one{ "one", {1,1} };
    robometry::ChannelInfo var_two{ "two", {1,1} };

    auto ok = bm.addChannel(var_one);
    ok = ok && bm.addChannel(var_two);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }
    std::cout << "Starting loop" << std::endl;
    for (int i = 0; i < 40; i++) {
        bm.push_back({ i }, "one");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bm.push_back({ i + 1 }, "two");
    }

    // now we use also the auto_saving option
    bufferConfig.auto_save = true;

    std::cout << "Second example: " << std::endl;

    robometry::BufferManager bm_m(bufferConfig);
    bm_m.setFileName("buffer_manager_test_matrix");
    std::vector<robometry::ChannelInfo> vars{ { "one",{2,3} },
                                   { "two",{3,2} } };

    ok = bm_m.addChannels(vars);
    if (!ok) {
        std::cout << "Problem adding variables...."<<std::endl;
        return 1;
    }

    for (int i = 0; i < 40; i++) {
        bm_m.push_back({ i + 1, i + 2, i + 3, i + 4, i + 5, i + 6 }, "one");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bm_m.push_back({ i * 1, i * 2, i * 3, i * 4, i * 5, i * 6 }, "two");
    }

    std::cout << "Third example: " << std::endl;

    bufferConfig.channels = { {"one",{4,1}}, {"two",{4,1}} };
    bufferConfig.filename = "buffer_manager_test_vector";

    robometry::BufferManager bm_v(bufferConfig);

    for (int i = 0; i < 40; i++) {
        bm_v.push_back({ i+1.0, i+2.0, i+3.0, i+4.0  }, "one");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        bm_v.push_back({ (double)i, i*2.0, i*3.0, i*4.0 }, "two");
    }

    return 0;
 }
