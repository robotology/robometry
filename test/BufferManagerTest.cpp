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
#include <catch2/catch_test_macros.hpp>
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

TEST_CASE("Diagnostic type names remain readable without Boost", "[buffer-manager]")
{
    REQUIRE(robometry::getTypeName<int>() == "int");
    REQUIRE(robometry::getTypeName(1.0) == "double");
    const auto vector_name = robometry::getTypeName<std::vector<int>>();
    REQUIRE(vector_name.find("vector<") != std::string::npos);
    REQUIRE(vector_name.find("int") != std::string::npos);

    struct Base { virtual ~Base() = default; };
    struct Derived : Base {};
    Derived derived;
    const Base& base = derived;
    REQUIRE(robometry::getTypeName(base) == robometry::getTypeName<Derived>());
}

TEST_CASE("Buffer Manager saves wrapped samples in chronological order", "[buffer-manager][boost-removal]")
{
    robometry::BufferConfig config;
    config.filename = "wrapped_buffer_regression";
    config.n_samples = 3;
    config.channels = {{"scalar", {1, 1}}, {"vector", {2, 1}}};
    robometry::BufferManager manager(config);
    for (int i = 0; i < 8; ++i)
    {
        manager.push_back(i, 0.5 * i, "scalar");
        manager.push_back(std::vector<double>{double(i), double(i + 10)}, 0.5 * i, "vector");
    }
    // Both attempts must be rejected without altering the saved sample count or values.
    manager.push_back(99.0, 99.0, "scalar");
    manager.push_back(std::vector<int>{99, 99}, 99.0, "vector");

    std::string path;
    REQUIRE(manager.saveToFile(path));
    matioCpp::File file(path + ".mat", matioCpp::FileMode::ReadOnly);
    REQUIRE(file.isOpen());
    auto root = file.read(config.filename).asStruct();
    auto scalar = root["scalar"].asStruct();
    auto scalar_data = scalar["data"].asMultiDimensionalArray<int>();
    auto timestamps = scalar["timestamps"].asVector<double>();
    REQUIRE(scalar_data.numberOfElements() == 3);
    REQUIRE(timestamps.size() == 3);
    auto vector_data = root["vector"]["data"].asMultiDimensionalArray<double>();
    REQUIRE(vector_data.numberOfElements() == 6);
    for (size_t i = 0; i < 3; ++i)
    {
        CHECK(scalar_data[i] == int(i + 5));
        CHECK(timestamps[i] == 0.5 * (i + 5));
        CHECK(vector_data[2 * i] == double(i + 5));
        CHECK(vector_data[2 * i + 1] == double(i + 15));
    }
    file.close();
    REQUIRE(matioCpp::File::Delete(path + ".mat"));
}

TEST_CASE("Buffer Manager preserves channel types after flushing", "[buffer-manager][boost-removal]")
{
    robometry::BufferConfig config;
    config.filename = "channel_type_regression";
    config.n_samples = 3;
    config.channels = {{"channel", {1, 1}}};
    robometry::BufferManager manager(config);
    manager.push_back(1, 1.0, "channel");
    std::string path;
    REQUIRE(manager.saveToFile(path));
    REQUIRE(matioCpp::File::Delete(path + ".mat"));

    manager.push_back(2.0, 2.0, "channel");
    manager.push_back(3, 3.0, "channel");
    REQUIRE(manager.saveToFile(path));
    matioCpp::File file(path + ".mat", matioCpp::FileMode::ReadOnly);
    REQUIRE(file.isOpen());
    auto root = file.read(config.filename);
    auto channel = root["channel"];
    auto data = channel["data"].asMultiDimensionalArray<int>();
    auto timestamps = channel["timestamps"].asVector<double>();
    REQUIRE(data.numberOfElements() == 1);
    REQUIRE(data[0] == 3);
    REQUIRE(timestamps[0] == 3.0);
    file.close();
    REQUIRE(matioCpp::File::Delete(path + ".mat"));
}

TEST_CASE("Buffer Manager Test")
{
    SECTION("Test scalar")
    {
        // The inputs to the API are defined in the BufferConfig structure
        robometry::BufferConfig bufferConfig;

        // We use the default config, setting only the number of samples (no auto/periodic saving)
        bufferConfig.n_samples = n_samples;

        robometry::BufferManager bm(bufferConfig);
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

        robometry::BufferManager bm_m(bufferConfig);
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

        robometry::BufferManager bm_v;
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

        robometry::BufferManager bm_v;
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

        robometry::BufferManager bm_v;
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

        robometry::BufferManager bm;
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
        robometry::BufferManager bm;
        robometry::BufferConfig bufferConfig;
        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.description_list = { "Be", "Or not to be" };
        bufferConfig.channels = { {"one",{1,1}, {"One0"}, {"meters"}}, {"two",{1,1}, {"Two0"}, {"degrees"}} };
        bufferConfig.filename = "buffer_manager_test_conf_file";
        bufferConfig.n_samples = 20;
        bufferConfig.save_period = 1.0;
        bufferConfig.data_threshold = 10;
        bufferConfig.save_periodically = true;
        bufferConfig.enable_compression = true;
        bufferConfig.auto_save = true;

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
        REQUIRE(bufferConfig.channels[0].elements_names.size() == 1);
        REQUIRE(bufferConfig.channels[0].elements_names[0] == "One0");
        REQUIRE(bufferConfig.channels[0].units_of_measure[0] == "meters");
        REQUIRE(bufferConfig.channels[0].elements_names[0] == "One0");
        REQUIRE(bufferConfig.channels[1].elements_names.size() == 1);
        REQUIRE(bufferConfig.channels[1].elements_names[0] == "Two0");
        REQUIRE(bufferConfig.channels[1].units_of_measure[0] == "degrees");
        REQUIRE(bufferConfig.enable_compression == true);
        REQUIRE(bufferConfig.auto_save == true);

        REQUIRE(bm.configure(bufferConfig));

        std::cout << "Starting loop" << std::endl;
        for (int i = 0; i < 40; i++) {
            bm.push_back({ i }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1 }, "two");
        }

    }

    SECTION("Test resize") {
        robometry::BufferManager bm;
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
        robometry::BufferManager bm;
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
        robometry::BufferManager bm;
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
        robometry::BufferManager bm;
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

    SECTION("BufferManager as class member") {
        struct StructContainingBufferManager {
            robometry::BufferManager bm;
            robometry::BufferConfig bufferConfig;
        };

        StructContainingBufferManager strBm;
        strBm.bm.addChannel({ "int_channel", {1}});
        strBm.bufferConfig.n_samples = n_samples;
        strBm.bufferConfig.filename = "buffer_manager_test_multiple_types";
        strBm.bufferConfig.auto_save = true;

        REQUIRE(strBm.bm.configure(strBm.bufferConfig));
        for (int i = 0; i < 10; i++) {
            strBm.bm.push_back(i, "int_channel");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

    }

    SECTION("Callback")
    {
        bool called = false;

        {
            robometry::BufferManager bm;
            robometry::BufferConfig bufferConfig;
            bufferConfig.n_samples = n_samples;
            bufferConfig.filename = "buffer_manager_test_callback";
            bufferConfig.auto_save = true;

            REQUIRE(bm.addChannel({ "int_channel", {1}}));
            REQUIRE(bm.setSaveCallback(testCallback));
            REQUIRE(bm.setPreSaveCallback([&called](const robometry::SaveCallbackSaveMethod& /**method*/)
            {
                called = true;
                return true;
            }));
            REQUIRE(bm.configure(bufferConfig));

            for (int i = 0; i < 10; i++) {
                bm.push_back(i, "int_channel");
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        REQUIRE(called);
    }

    SECTION("Unit of measure") {
        robometry::BufferManager bm;
        robometry::BufferConfig bufferConfig;

        robometry::ChannelInfo var_one{ "one", {1,1} };
        robometry::ChannelInfo var_two{ "two", {1,1} };
        var_one.units_of_measure = {"m"};
        var_two.units_of_measure = {"deg"};
        // First add channels that will be handling empty buffers
        REQUIRE(bm.addChannel(var_one));
        REQUIRE(bm.addChannel(var_two));

        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.description_list = { "Be", "Or not to be" };
        bufferConfig.filename = "buffer_manager_test_unit_measure";
        bufferConfig.n_samples = n_samples;
        REQUIRE(bm.configure(bufferConfig));

        for (int i = 0; i < 3; i++) {
            bm.push_back({ i }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1 }, "two");
        }
        // Check manual save
        REQUIRE(bm.saveToFile());

    }

    SECTION("Unit of measure vector") {
        robometry::BufferManager bm;
        robometry::BufferConfig bufferConfig;

        robometry::ChannelInfo var_one{ "one", {2,1}, {}, {"m"} };
        robometry::ChannelInfo var_two{ "two", {2,1}, {}, {"deg"} };
        // First add channels that will be handling empty buffers
        REQUIRE(bm.addChannel(var_one));
        REQUIRE(bm.addChannel(var_two));

        bufferConfig.yarp_robot_name = "robot";
        bufferConfig.description_list = { "Be", "Or not to be" };
        bufferConfig.filename = "buffer_manager_test_unit_measure_vector";
        bufferConfig.n_samples = n_samples;
        REQUIRE(bm.configure(bufferConfig));

        for (int i = 0; i < 3; i++) {
            bm.push_back({ i, i + 1 }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1, i + 2 }, "two");
        }
        // Check manual save
        REQUIRE(bm.saveToFile());

    }

    SECTION("Test clear") {
        robometry::BufferManager bm;
        robometry::BufferConfig bufferConfig;

        robometry::ChannelInfo var_one{ "one", {1,1} };
        robometry::ChannelInfo var_two{ "two", {1,1} };

        // Add channels
        REQUIRE(bm.addChannel(var_one));
        REQUIRE(bm.addChannel(var_two));

        bufferConfig.filename = "buffer_manager_test_clear";
        bufferConfig.n_samples = n_samples;
        REQUIRE(bm.configure(bufferConfig));

        // Add some data
        for (int i = 0; i < 3; i++) {
            bm.push_back({ i }, "one");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            bm.push_back({ i + 1 }, "two");
        }

        // Clear the buffer manager
        bm.clear();

        // Verify channels were cleared - the config should be empty
        auto config_after = bm.getBufferConfig();
        REQUIRE(config_after.channels.empty());

        // Verify we can reconfigure after clear
        bufferConfig.filename = "buffer_manager_test_clear_after";
        REQUIRE(bm.configure(bufferConfig));
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
            robometry::BufferManager bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };
        bufferConfig.n_samples = 10000;
        BENCHMARK_ADVANCED("BufferOfInt-10000Samples-oneVariable-1x1")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

        bufferConfig.n_samples = 100000;
        BENCHMARK_ADVANCED("BufferOfInt-100000Samples-oneVariable-1x1")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager bm;
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
            robometry::BufferManager bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i, i + 1, i + 2 }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

        bufferConfig.n_samples = 10000;
        BENCHMARK_ADVANCED("BufferOfInt-10000Samples-oneVariable-3x1")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i, i + 1, i + 2 }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

        bufferConfig.n_samples = 100000;
        BENCHMARK_ADVANCED("BufferOfInt-100000Samples-oneVariable-3x1")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager bm;
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
            robometry::BufferManager bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i, i + 1, i + 2, i + 3, i + 4, i + 5 }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

        bufferConfig.n_samples = 10000;
        BENCHMARK_ADVANCED("BufferOfInt-10000Samples-oneVariable-3x2")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager bm;
            bm.configure(bufferConfig);

            for (int i = 0; i < bufferConfig.n_samples; i++) {
                bm.push_back({ i, i + 1, i + 2, i + 3, i + 4, i + 5 }, "one");
            }
            // measure
            meter.measure([&bm] { return bm.saveToFile(); });
        };

        bufferConfig.n_samples = 100000;
        BENCHMARK_ADVANCED("BufferOfInt-100000Samples-oneVariable-3x2")(Catch::Benchmark::Chronometer meter) {
            robometry::BufferManager bm;
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
