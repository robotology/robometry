/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */
// This has to be probably removed when we will have multiple tests
#define CATCH_CONFIG_MAIN

#include <yarp/telemetry/BufferManager.h>
#include <yarp/os/Network.h>
#include <yarp/os/Time.h>
#include <catch2/catch.hpp>
#include <vector>

constexpr size_t n_samples{ 3 };

TEST_CASE("Buffer Manager Test")
{
    yarp::os::Network yarp;
    auto now = yarp::os::Time::now;
    SECTION("Test scalar")
    {
        // The inputs to the API are defined in the BufferConfig structure
        yarp::telemetry::BufferConfig bufferConfig;

        // We use the default config, setting only the number of samples (no auto/periodic saving)
        bufferConfig.n_samples = n_samples;

        yarp::telemetry::BufferManager<int32_t> bm(bufferConfig);
        bm.setFileName("buffer_manager_test");
        auto ok = bm.setNowFunction(now);
        // Check that the now function has been set correctly.
        REQUIRE(ok);

        yarp::telemetry::ChannelInfo var_one{ "one", {1,1} };
        yarp::telemetry::ChannelInfo var_two{ "two", {1,1} };

        ok = bm.addChannel(var_one);
        // Check that the channel one has been correctly added
        REQUIRE(ok);
        ok = bm.addChannel(var_two);
        // Check that the channel two has been correctly added
        REQUIRE(ok);

        for (int i = 0; i < 3; i++) {
            bm.push_back({ i }, "one");
            yarp::os::Time::delay(0.01);
            bm.push_back({ i + 1 }, "two");
        }
        // Check manual save
        REQUIRE(bm.saveToFile());

    }
    SECTION("Test matrix") {
        // The inputs to the API are defined in the BufferConfig structure
        yarp::telemetry::BufferConfig bufferConfig;

        // We use the default config, setting only the number of samples (no auto/periodic saving)
        bufferConfig.n_samples = n_samples;
        // now we test our API with the auto_save option enabled.
        bufferConfig.auto_save = true;

        yarp::telemetry::BufferManager<int32_t> bm_m(bufferConfig);
        bm_m.setFileName("buffer_manager_test_matrix");

        std::vector<yarp::telemetry::ChannelInfo> vars{ { "one",{2,3} },
                                                        { "two",{3,2} } };

        REQUIRE(bm_m.addChannels(vars));

        for (int i = 0; i < 10; i++) {
            bm_m.push_back({ i + 1, i + 2, i + 3, i + 4, i + 5, i + 6 }, "one");
            yarp::os::Time::delay(0.01);
            bm_m.push_back({ i * 1, i * 2, i * 3, i * 4, i * 5, i * 6 }, "two");
        }
    }

    SECTION("Test vector") {
        // The inputs to the API are defined in the BufferConfig structure
        yarp::telemetry::BufferConfig bufferConfig;

        // We use the default config, setting only the number of samples (no auto/periodic saving)
        bufferConfig.n_samples = n_samples;
        bufferConfig.channels = { {"one",{4,1}}, {"two",{4,1}} };
        bufferConfig.filename = "buffer_manager_test_vector";
        bufferConfig.auto_save = true;

        yarp::telemetry::BufferManager<double> bm_v;
        REQUIRE(bm_v.configure(bufferConfig));

        for (int i = 0; i < 10; i++) {
            bm_v.push_back({ i + 1.0, i + 2.0, i + 3.0, i + 4.0 }, "one");
            yarp::os::Time::delay(0.01);
            bm_v.push_back({ (double)i, i * 2.0, i * 3.0, i * 4.0 }, "two");
        }
    }

    SECTION("Test periodic save") {

        yarp::telemetry::BufferConfig bufferConfig;

        // we configure our API to use our periodic saving option
        bufferConfig.n_samples = 20;
        bufferConfig.data_threshold = 10;
        bufferConfig.auto_save = true;

        yarp::telemetry::BufferManager<int32_t> bm;
        REQUIRE(bm.configure(bufferConfig));
        bm.setFileName("buffer_manager_test_periodic");
        yarp::telemetry::ChannelInfo var_one{ "one", {1,1} };
        yarp::telemetry::ChannelInfo var_two{ "two", {1,1} };

        REQUIRE(bm.addChannel(var_one));
        REQUIRE(bm.addChannel(var_two));

        REQUIRE(bm.enablePeriodicSave(0.1));
        for (int i = 0; i < 40; i++) {
            bm.push_back({ i }, "one");
            yarp::os::Time::delay(0.01);
            bm.push_back({ i + 1 }, "two");
        }
    }

    SECTION("Test configuration from/to file") {
        yarp::telemetry::BufferManager<int32_t> bm;
        yarp::telemetry::BufferConfig bufferConfig;
        bufferConfig.description_list = { "Be", "Or not to be" };
        bufferConfig.channels = { {"one",{1,1}}, {"two",{1,1}} };
        bufferConfig.filename = "buffer_manager_test_conf_file";
        bufferConfig.n_samples = 20;
        bufferConfig.save_period = 1.0;
        bufferConfig.data_threshold = 10;
        bufferConfig.save_periodically = true;

        REQUIRE(bufferConfigToJson(bufferConfig, "test_json_write.json"));

        bool ok = bufferConfigFromJson(bufferConfig, "test_json_write.json");
        REQUIRE(ok);

        REQUIRE(bufferConfig.description_list.size() == 2);
        REQUIRE(bufferConfig.description_list[0] == "Be");
        REQUIRE(bufferConfig.description_list[1] == "Or not to be");
        REQUIRE(bufferConfig.filename == "buffer_manager_test_conf_file");
        REQUIRE(bufferConfig.n_samples == 20);
        REQUIRE(bufferConfig.save_period == 1.0);
        REQUIRE(bufferConfig.data_threshold == 10);
        REQUIRE(bufferConfig.save_periodically == true);
        REQUIRE(bufferConfig.channels.size() == 2);
        REQUIRE(bufferConfig.channels[0].first == "one");
        REQUIRE(bufferConfig.channels[0].second == std::vector<size_t>{1, 1});
        REQUIRE(bufferConfig.channels[1].first == "two");
        REQUIRE(bufferConfig.channels[1].second == std::vector<size_t>{1, 1});

        REQUIRE(bm.configure(bufferConfig));

        std::cout << "Starting loop" << std::endl;
        for (int i = 0; i < 40; i++) {
            bm.push_back({ i }, "one");
            yarp::os::Time::delay(0.01);
            bm.push_back({ i + 1 }, "two");
        }

    }

    SECTION("Test resize") {
        yarp::telemetry::BufferManager<int32_t> bm;
        yarp::telemetry::BufferConfig bufferConfig;

        yarp::telemetry::ChannelInfo var_one{ "one", {1,1} };
        yarp::telemetry::ChannelInfo var_two{ "two", {1,1} };
        // First add channels that will be handling empty buffers
        REQUIRE(bm.addChannel(var_one));
        REQUIRE(bm.addChannel(var_two));

        bufferConfig.description_list = { "Be", "Or not to be" };
        bufferConfig.filename = "buffer_manager_test_resize";
        bufferConfig.n_samples = 20;
        bufferConfig.data_threshold = 10;
        bufferConfig.save_periodically = false;

        // This will resize the buffers
        REQUIRE(bm.configure(bufferConfig));

        for (int i = 0; i < 40; i++) {
            bm.push_back({ i }, "one");
            yarp::os::Time::delay(0.01);
            bm.push_back({ i + 1 }, "two");
        }

        REQUIRE(bm.saveToFile());

    }

}
