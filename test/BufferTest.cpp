/*
 * Copyright (C) 2026 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <robometry/Buffer.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Buffer container aliases support clients sharing storage beyond its owner", "[buffer]")
{
    robometry::Buffer::shared_buffer_type storage;
    {
        robometry::Buffer buffer(2);
        storage = buffer.getBufferSharedPtr();
        buffer.push_back({1.0, 10});
    }
    storage->push_back({2.0, 20});
    REQUIRE(storage->size() == 2);
    REQUIRE(storage->front().m_ts == 1.0);
    REQUIRE(std::any_cast<int>(storage->back().m_datum) == 20);

    robometry::Buffer::container_type snapshot = *storage;
    snapshot.clear();
    REQUIRE(storage->size() == 2);
}

TEST_CASE("Buffer copies and shared pointers retain shared storage", "[buffer]")
{
    robometry::Buffer buffer(2);
    const robometry::Record first{1.0, 10};
    buffer.push_back(first);
    buffer.push_back({2.0, 20});
    auto copy = buffer;
    copy.push_back({3.0, 30});
    REQUIRE(buffer.size() == 2);
    REQUIRE(buffer.capacity() == 2);
    REQUIRE(buffer.full());
    REQUIRE(buffer.getBufferFreeSpace() == 0);
    const auto& const_buffer = buffer;
    REQUIRE(const_buffer.begin()->m_ts == 2.0);
    REQUIRE(std::any_cast<int>(const_buffer.begin()->m_datum) == 20);

    auto storage = buffer.getBufferSharedPtr();
    REQUIRE(storage == copy.getBufferSharedPtr());
    storage->pop_back();
    REQUIRE(buffer.size() == 1);
    REQUIRE(buffer.getBufferFreeSpace() == 1);
    copy.clear();
    REQUIRE(buffer.empty());
    buffer.set_capacity(3);
    REQUIRE(copy.capacity() == 3);
    buffer.resize(2);
    REQUIRE(copy.size() == 2);
    REQUIRE(buffer.begin()->m_ts == 0.0);
    REQUIRE_FALSE(buffer.begin()->m_datum.has_value());
    REQUIRE(std::distance(const_buffer.begin(), const_buffer.end()) == 2);
}

TEST_CASE("Default Buffer is an empty usable zero-capacity buffer", "[buffer]")
{
    robometry::Buffer buffer;
    REQUIRE(buffer.empty());
    REQUIRE(buffer.full());
    REQUIRE(buffer.capacity() == 0);
    REQUIRE(buffer.begin() == buffer.end());
    buffer.push_back({1.0, 1});
    REQUIRE(buffer.empty());
    buffer.set_capacity(1);
    buffer.push_back({2.0, 2});
    REQUIRE(buffer.size() == 1);
    buffer.clear();
    REQUIRE(buffer.empty());
}

TEST_CASE("Buffer releases std::any payloads when cleared", "[buffer]")
{
    robometry::Buffer buffer(1);
    auto payload = std::make_shared<int>(1);
    std::weak_ptr<int> weak = payload;
    buffer.push_back({1.0, std::move(payload)});
    REQUIRE_FALSE(weak.expired());
    buffer.clear();
    REQUIRE(weak.expired());
    REQUIRE(buffer.capacity() == 1);
}
