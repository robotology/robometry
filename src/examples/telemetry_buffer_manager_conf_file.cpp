/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */


#include <matioCpp/ForwardDeclarations.h>
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
constexpr auto file_indexing = "%Y_%m_%d_%H_%M_%S";


int main()
{
    robometry::BufferConfig bufferConfig;

    // we configure our API to use our periodic saving option
    bufferConfig.filename = "test_json_write";
    bufferConfig.n_samples = n_samples;
    bufferConfig.save_period = check_period;
    bufferConfig.data_threshold = threshold;
    bufferConfig.save_periodically = true;
    bufferConfig.file_indexing = file_indexing;
    bufferConfig.mat_file_version = matioCpp::FileVersion::MAT7_3;

    std::vector<robometry::ChannelInfo> vars{ { "one",{2,3} },
                                                    { "two",{3,2} } };
    bufferConfig.channels = vars;

    auto ok = bufferConfigToJson(bufferConfig, "test_json_write.json");

    if (!ok) {
        std::cout << "Problems saving configuration to json" << std::endl;
        return 1;
    }

    robometry::BufferManager<int32_t> bm;

    ok = bufferConfigFromJson(bufferConfig,"test_json_write.json");

    ok = ok && bm.configure(bufferConfig);

    if (!ok) {
        std::cout << "Problems configuring from file" << std::endl;
        return 1;
    }

    std::cout << "Starting loop" << std::endl;
    for (int i = 0; i < 40; i++) {
        bm.push_back({ i }, "one");
        bm.push_back({ i + 1 }, "one");
        bm.push_back(std::vector<int>{ i + 2 }, "one");
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
 }
