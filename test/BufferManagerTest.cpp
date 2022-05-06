/*
 * Copyright (C) 2006-2020 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */
// This has to be probably removed when we will have multiple tests
#define CATCH_CONFIG_MAIN

#include <robometry/BufferManager.h>
#include <catch2/catch.hpp>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>

constexpr size_t n_samples{ 3 };

bool testCallback(const std::string& file_name,
                  const robometry::SaveCallbackSaveMethod& /**method */) {
    std::string file_name_with_extension = file_name + ".txt";
    std::ofstream my_file(file_name_with_extension.c_str());

    // Write to the file
    my_file << "Dummy file!";

    // Close the file
    my_file.close();

    return true;
};
struct testStruct
{
    int a;
    double b;
};
VISITABLE_STRUCT(testStruct, a, b);

TEST_CASE("Buffer Manager Test")
{
    SECTION("Test scalar")
    {
        // The inputs to the API are defined in the BufferConfig structure
        robometry::BufferConfig bufferConfig;

        // We use the default config, setting only the number of samples (no auto/periodic saving)
        bufferConfig.n_samples = n_samples;

        robometry::BufferManager<int32_t> bm(bufferConfig);
        bm.setFileName("buffer_manager_test");

        robometry::ChannelInfo var_one{ "one", {1,1} };
        robometry::ChannelInfo var_two{ "two", {1,1} };

        bool ok {false};
        ok = bm.addChannel(var_one);
        // Check that the channel one has been correctly added
        REQUIRE(ok);
        ok = bm.addChannel(var_two);
        // Check that the channel two has been correctly added
        REQUIRE(ok);

        for (int i = 0; i < 3; i++) {
            bm.push_back({ i }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1 }, "two");
        }
        // Check manual save
        REQUIRE(bm.saveToFile());

    }
    SECTION("Test matrix") {
        // The inputs to the API are defined in the BufferConfig structure
        robometry::BufferConfig bufferConfig;

        // We use the default config, setting only the number of samples (no auto/periodic saving)
        bufferConfig.n_samples = n_samples;
        // now we test our API with the auto_save option enabled.
        bufferConfig.auto_save = true;

        robometry::BufferManager<int32_t> bm_m(bufferConfig);
        bm_m.setFileName("buffer_manager_test_matrix");

        std::vector<robometry::ChannelInfo> vars{ { "one",{2,3} },
                                                        { "two",{3,2} } };

        REQUIRE(bm_m.addChannels(vars));

        for (int i = 0; i < 10; i++) {
            bm_m.push_back({ i + 1, i + 2, i + 3, i + 4, i + 5, i + 6 }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm_m.push_back({ i * 1, i * 2, i * 3, i * 4, i * 5, i * 6 }, "two");
        }
    }

    SECTION("Test vector") {
        // The inputs to the API are defined in the BufferConfig structure
        robometry::BufferConfig bufferConfig;

        // We use the default config, setting only the number of samples (no auto/periodic saving)
        bufferConfig.n_samples = n_samples;
        bufferConfig.channels = { {"one",{4,1}}, {"two",{4,1}} };
        bufferConfig.filename = "buffer_manager_test_vector";
        bufferConfig.auto_save = true;

        robometry::BufferManager<double> bm_v;
        REQUIRE(bm_v.configure(bufferConfig));

        for (int i = 0; i < 10; i++) {
            bm_v.push_back({ i + 1.0, i + 2.0, i + 3.0, i + 4.0 }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm_v.push_back({ (double)i, i * 2.0, i * 3.0, i * 4.0 }, "two");
        }
    }

    SECTION("Test vector with elements names") {
        // The inputs to the API are defined in the BufferConfig structure
        robometry::BufferConfig bufferConfig;

        // We use the default config, setting only the number of samples (no auto/periodic saving)
        bufferConfig.n_samples = n_samples;
        bufferConfig.channels = { {"one", {4,1}, {"e0", "e1", "e2", "e3"}},
                                  {"two", {4,1}, {"v0", "v1", "v2", "v3"}} };
        bufferConfig.filename = "buffer_manager_test_vector";
        bufferConfig.auto_save = true;

        robometry::BufferManager<double> bm_v;
        REQUIRE(bm_v.configure(bufferConfig));

        for (int i = 0; i < 10; i++) {
            bm_v.push_back({ i + 1.0, i + 2.0, i + 3.0, i + 4.0 }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm_v.push_back({ (double)i, i * 2.0, i * 3.0, i * 4.0 }, "two");
        }
    }

    SECTION("Test nested vector") {
        // The inputs to the API are defined in the BufferConfig structure
        robometry::BufferConfig bufferConfig;

        // We use the default config, setting only the number of samples (no auto/periodic saving)
        bufferConfig.n_samples = n_samples;
        bufferConfig.channels = { {"struct1::one",{4,1}},
                                  {"struct1::two",{4,1}},
                                  {"struct2::one",{4,1}} };
        bufferConfig.filename = "buffer_manager_test_nested_vector";
        bufferConfig.auto_save = true;

        robometry::BufferManager<double> bm_v;
        REQUIRE(bm_v.configure(bufferConfig));

        for (int i = 0; i < 10; i++) {
            bm_v.push_back({ i+1.0, i+2.0, i+3.0, i+4.0  }, "struct1::one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm_v.push_back({ (double)i, i*2.0, i*3.0, i*4.0 }, "struct1::two");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm_v.push_back({ (double)i, i/2.0, i/3.0, i/4.0 }, "struct2::one");
        }
    }

    SECTION("Test periodic save") {

        robometry::BufferConfig bufferConfig;

        // we configure our API to use our periodic saving option
        bufferConfig.n_samples = 20;
        bufferConfig.data_threshold = 10;
        bufferConfig.auto_save = true;

        robometry::BufferManager<int32_t> bm;
        REQUIRE(bm.configure(bufferConfig));
        bm.setFileName("buffer_manager_test_periodic");
        robometry::ChannelInfo var_one{ "one", {1,1} };
        robometry::ChannelInfo var_two{ "two", {1,1} };

        REQUIRE(bm.addChannel(var_one));
        REQUIRE(bm.addChannel(var_two));

        REQUIRE(bm.enablePeriodicSave(0.1));
        for (int i = 0; i < 40; i++) {
            bm.push_back({ i }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1 }, "two");
        }
    }

    SECTION("Test configuration from/to file") {
        robometry::BufferManager<int32_t> bm;
        robometry::BufferConfig bufferConfig;
        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.description_list = { "Be", "Or not to be" };
        bufferConfig.channels = { {"one",{1,1}}, {"two",{1,1}} };
        bufferConfig.filename = "buffer_manager_test_conf_file";
        bufferConfig.n_samples = 20;
        bufferConfig.save_period = 1.0;
        bufferConfig.data_threshold = 10;
        bufferConfig.save_periodically = true;
        bufferConfig.enable_compression = true;

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
        REQUIRE(bufferConfig.channels[0].name == "one");
        REQUIRE(bufferConfig.channels[0].dimensions == std::vector<size_t>{1, 1});
        REQUIRE(bufferConfig.channels[1].name == "two");
        REQUIRE(bufferConfig.channels[1].dimensions == std::vector<size_t>{1, 1});
        REQUIRE(bufferConfig.enable_compression == true);

        REQUIRE(bm.configure(bufferConfig));

        std::cout << "Starting loop" << std::endl;
        for (int i = 0; i < 40; i++) {
            bm.push_back({ i }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1 }, "two");
        }

    }

    SECTION("Test resize") {
        robometry::BufferManager<int32_t> bm;
        robometry::BufferConfig bufferConfig;

        robometry::ChannelInfo var_one{ "one", {1,1} };
        robometry::ChannelInfo var_two{ "two", {1,1} };
        // First add channels that will be handling empty buffers
        REQUIRE(bm.addChannel(var_one));
        REQUIRE(bm.addChannel(var_two));

        bufferConfig.description_list = { "Be", "Or not to be" };
        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.filename = "buffer_manager_test_resize";
        bufferConfig.data_threshold = 10;
        bufferConfig.save_periodically = false;

        REQUIRE(bm.configure(bufferConfig));

        // Test resize
        bm.resize(20);

        for (int i = 0; i < 40; i++) {
            bm.push_back({ i }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1 }, "two");
        }

        REQUIRE(bm.saveToFile());

    }

    SECTION("Test very long period") {
        robometry::BufferManager<int32_t> bm;
        robometry::BufferConfig bufferConfig;

        robometry::ChannelInfo var_one{ "one", {1,1} };
        robometry::ChannelInfo var_two{ "two", {1,1} };
        // First add channels that will be handling empty buffers
        REQUIRE(bm.addChannel(var_one));
        REQUIRE(bm.addChannel(var_two));

        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.description_list = { "Be", "Or not to be" };
        bufferConfig.filename = "buffer_manager_test_long_period";
        bufferConfig.n_samples = 20;
        bufferConfig.data_threshold = 10;
        bufferConfig.save_periodically = true;
        bufferConfig.auto_save = true;
        bufferConfig.save_period = 3600; // Save every hour.
        bufferConfig.enable_compression = true;

        // This will set_capacity the buffers
        REQUIRE(bm.configure(bufferConfig));

        for (int i = 0; i < 40; i++) {
            bm.push_back({ i }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1 }, "two");
        }

    }

    SECTION("Test set_capacity") {
        robometry::BufferManager<int32_t> bm;
        robometry::BufferConfig bufferConfig;

        robometry::ChannelInfo var_one{ "one", {1,1} };
        robometry::ChannelInfo var_two{ "two", {1,1} };
        // First add channels that will be handling empty buffers
        REQUIRE(bm.addChannel(var_one));
        REQUIRE(bm.addChannel(var_two));

        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.description_list = { "Be", "Or not to be" };
        bufferConfig.filename = "buffer_manager_test_set_capacity";
        bufferConfig.n_samples = 20;
        bufferConfig.data_threshold = 10;
        bufferConfig.save_periodically = false;

        // This will resize the buffers
        REQUIRE(bm.configure(bufferConfig));

        // Try to allocate a bigger buffer than what we need
        bm.set_capacity(2000);

        for (int i = 0; i < 40; i++) {
            bm.push_back({ i }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1 }, "two");
        }

        REQUIRE(bm.saveToFile());

    }

    SECTION("Test path existence") {
        robometry::BufferManager<int32_t> bm;
        robometry::BufferConfig bufferConfig;

        robometry::ChannelInfo var_one{ "one", {1,1} };
        robometry::ChannelInfo var_two{ "two", {1,1} };
        // First add channels that will be handling empty buffers
        REQUIRE(bm.addChannel(var_one));
        REQUIRE(bm.addChannel(var_two));

        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.description_list = { "Be", "Or not to be" };
        bufferConfig.filename = "buffer_manager_test_path_existence";
        bufferConfig.n_samples = 20;
        bufferConfig.data_threshold = 10;
        bufferConfig.auto_save = true;
        bufferConfig.path = "/this/path/does/not/exist";

        REQUIRE(!bm.configure(bufferConfig));

        bufferConfig.path = "./";
        REQUIRE(bm.configure(bufferConfig));

        for (int i = 0; i < 40; i++) {
            bm.push_back({ i }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1 }, "two");
        }

    }

    SECTION("Multiple types") {

        robometry::BufferManager bm;
        robometry::BufferConfig bufferConfig;

        robometry::ChannelInfo var_int{ "int_channel", {1}};
        robometry::ChannelInfo var_double{ "double_channel", {1}};
        robometry::ChannelInfo var_string{ "string_channel", {1}};
        robometry::ChannelInfo var_vector{ "vector_channel", {4, 1}};
        robometry::ChannelInfo var_struct{ "struct_channel", {1}};

        REQUIRE(bm.addChannel(var_int));
        REQUIRE(bm.addChannel(var_double));
        REQUIRE(bm.addChannel(var_string));
        REQUIRE(bm.addChannel(var_vector));
        REQUIRE(bm.addChannel(var_struct));

        bufferConfig.n_samples = n_samples;
        bufferConfig.filename = "buffer_manager_test_multiple_types";
        bufferConfig.auto_save = true;

        REQUIRE(bm.configure(bufferConfig));

        testStruct item;

        for (int i = 0; i < 10; i++) {
            bm.push_back(i, "int_channel");
            bm.push_back(i * 1.0, "double_channel");
            bm.push_back("iter" + std::to_string(i), "string_channel");
            bm.push_back({i + 0.0, i + 1.0, i + 2.0, i + 3.0}, "vector_channel");
            item.a = i;
            item.b = i;
            bm.push_back(item, "struct_channel");

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    SECTION("Callback")
    {
        robometry::BufferManager bm;
        robometry::BufferConfig bufferConfig;
        bufferConfig.n_samples = n_samples;
        bufferConfig.filename = "buffer_manager_test_callback";
        bufferConfig.auto_save = true;

        REQUIRE(bm.addChannel({ "int_channel", {1}}));
        bm.setSaveCallback(testCallback);
        REQUIRE(bm.configure(bufferConfig));

        for (int i = 0; i < 10; i++) {
            bm.push_back(i, "int_channel");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

#if defined CATCH_CONFIG_ENABLE_BENCHMARKING

    SECTION("Benchmarking section scalar int") {
        robometry::BufferConfig bufferConfig;
        robometry::ChannelInfo var_one{ "one", {1,1} };
        bufferConfig.channels.push_back(var_one);
        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.filename = "buffer_manager_test_scalar_benchmark";

        bufferConfig.n_samples = 1000;
        BENCHMARK_ADVANCED("BufferOfInt-1000Samples-oneVariable-1x1")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager<int32_t> bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };
        bufferConfig.n_samples = 10000;
        BENCHMARK_ADVANCED("BufferOfInt-10000Samples-oneVariable-1x1")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager<int32_t> bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

        bufferConfig.n_samples = 100000;
        BENCHMARK_ADVANCED("BufferOfInt-100000Samples-oneVariable-1x1")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager<int32_t> bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

    }
    SECTION("Benchmarking section vector int") {

        robometry::BufferConfig bufferConfig;
        robometry::ChannelInfo var_one{ "one", {3,1} };
        bufferConfig.channels.push_back(var_one);
        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.filename = "buffer_manager_test_vector_benchmark";


        bufferConfig.n_samples = 1000;
        BENCHMARK_ADVANCED("BufferOfInt-1000Samples-oneVariable-3x1")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager<int32_t> bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i, i + 1, i + 2 }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

        bufferConfig.n_samples = 10000;
        BENCHMARK_ADVANCED("BufferOfInt-10000Samples-oneVariable-3x1")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager<int32_t> bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i, i + 1, i + 2 }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

        bufferConfig.n_samples = 100000;
        BENCHMARK_ADVANCED("BufferOfInt-100000Samples-oneVariable-3x1")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager<int32_t> bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i, i + 1, i + 2 }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

    }


    SECTION("Benchmarking section matrix int") {

        robometry::BufferConfig bufferConfig;
        robometry::ChannelInfo var_one{ "one", {3,2} };
        bufferConfig.channels.push_back(var_one);
        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.filename = "buffer_manager_test_matrix_benchmark";


        bufferConfig.n_samples = 1000;
        BENCHMARK_ADVANCED("BufferOfInt-1000Samples-oneVariable-3x2")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager<int32_t> bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i, i + 1, i + 2, i + 3, i + 4, i + 5 }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

        bufferConfig.n_samples = 10000;
        BENCHMARK_ADVANCED("BufferOfInt-10000Samples-oneVariable-3x2")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager<int32_t> bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i, i + 1, i + 2, i + 3, i + 4, i + 5 }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

        bufferConfig.n_samples = 100000;
        BENCHMARK_ADVANCED("BufferOfInt-100000Samples-oneVariable-3x2")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager<int32_t> bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i, i + 1, i + 2, i + 3, i + 4, i + 5 }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };
    }
#endif // CATCH_CONFIG_ENABLE_BENCHMARKING
}
