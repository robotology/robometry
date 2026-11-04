/*
 * Copyright (C) 2026 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <robometry/CircularBuffer.h>
#include <catch2/catch_test_macros.hpp>

#include <deque>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

using robometry::CircularBuffer;

TEST_CASE("Circular buffer handles zero and unit capacity", "[circular-buffer]")
{
    CircularBuffer<int> buffer;
    REQUIRE(buffer.empty());
    REQUIRE(buffer.full());
    REQUIRE(buffer.begin() == buffer.end());
    buffer.push_back(1);
    REQUIRE(buffer.empty());

    buffer.set_capacity(1);
    buffer.push_back(1);
    buffer.push_back(2);
    REQUIRE(buffer.size() == 1);
    REQUIRE(buffer.front() == 2);
    REQUIRE(buffer.back() == 2);
    buffer.pop_front();
    buffer.push_back(3);
    REQUIRE(buffer.front() == 3);
    buffer.pop_back();
    REQUIRE(buffer.empty());
    buffer.clear();
    buffer.set_capacity(0);
    buffer.push_back(4);
    REQUIRE(buffer.empty());
}

TEST_CASE("Circular buffer overwrites oldest elements and reuses storage", "[circular-buffer]")
{
    CircularBuffer<int> buffer(3);
    for (int i = 0; i < 3; ++i)
        buffer.push_back(i);
    std::vector<const int*> slots;
    for (const auto& value : buffer)
        slots.push_back(&value);

    for (int i = 3; i < 100; ++i)
    {
        buffer.push_back(i);
        REQUIRE(std::find(slots.begin(), slots.end(), &buffer.back()) != slots.end());
    }
    REQUIRE(std::vector<int>(buffer.begin(), buffer.end()) == std::vector<int>{97, 98, 99});
    buffer.pop_back();
    buffer.pop_front();
    REQUIRE(buffer.front() == 98);
    buffer.push_back(100);
    buffer.push_back(101);
    REQUIRE(std::vector<int>(buffer.begin(), buffer.end()) == std::vector<int>{98, 100, 101});
}

TEST_CASE("Circular buffer provides mutable and const random access iterators", "[circular-buffer]")
{
    static_assert(std::is_convertible_v<CircularBuffer<int>::iterator, CircularBuffer<int>::const_iterator>);
    static_assert(!std::is_convertible_v<CircularBuffer<int>::const_iterator, CircularBuffer<int>::iterator>);
    static_assert(std::is_same_v<decltype(*std::declval<CircularBuffer<int>::const_iterator>()), const int&>);
    CircularBuffer<int> buffer(3);
    for (int value : {0, 3, 1, 2})
        buffer.push_back(value);
    const auto& const_buffer = buffer;
    REQUIRE(const_buffer.end() - buffer.begin() == 3);
    REQUIRE(buffer.end() - const_buffer.begin() == 3);
    REQUIRE(buffer.begin() == const_buffer.begin());
    REQUIRE(const_buffer.begin() == buffer.begin());
    REQUIRE(buffer.begin() < const_buffer.end());
    REQUIRE(buffer.end() > const_buffer.begin());
    REQUIRE(buffer.begin() <= const_buffer.begin());
    REQUIRE(buffer.end() >= const_buffer.end());
    REQUIRE(2 + buffer.begin() == buffer.end() - 1);
    REQUIRE(buffer.begin()[1] == 1);
    auto it = buffer.begin();
    REQUIRE(*it++ == 3);
    REQUIRE(*it-- == 1);
    REQUIRE(*++it == 1);
    REQUIRE(*--it == 3);
    it += 3;
    it -= 2;
    REQUIRE(*it == 1);
    std::sort(buffer.begin(), buffer.end());
    REQUIRE(std::vector<int>(const_buffer.cbegin(), const_buffer.cend()) == std::vector<int>{1, 2, 3});
    REQUIRE(*std::make_reverse_iterator(const_buffer.end()) == 3);
    REQUIRE(*buffer.begin().operator->() == 1);
    REQUIRE(std::distance(buffer.begin(), buffer.end()) == 3);
    REQUIRE(CircularBuffer<int>::iterator{} == CircularBuffer<int>::iterator{});
}

TEST_CASE("Circular buffer resizing preserves oldest elements", "[circular-buffer]")
{
    CircularBuffer<int> buffer(4);
    for (int i = 1; i <= 6; ++i)
        buffer.push_back(i);
    buffer.set_capacity(2);
    REQUIRE(std::vector<int>(buffer.begin(), buffer.end()) == std::vector<int>{3, 4});
    buffer.set_capacity(5);
    REQUIRE(buffer.size() == 2);
    REQUIRE(buffer.capacity() == 5);
    buffer.resize(4, 9);
    REQUIRE(std::vector<int>(buffer.begin(), buffer.end()) == std::vector<int>{3, 4, 9, 9});
    buffer.resize(1);
    REQUIRE(buffer.front() == 3);
    REQUIRE(buffer.capacity() == 5);
    buffer.resize(6, buffer.front());
    REQUIRE(buffer.capacity() == 6);
    REQUIRE(std::vector<int>(buffer.begin(), buffer.end()) == std::vector<int>(6, 3));
    buffer.resize(0);
    REQUIRE(buffer.capacity() == 6);
    buffer.resize(2);
    REQUIRE(std::vector<int>(buffer.begin(), buffer.end()) == std::vector<int>{0, 0});
    buffer.set_capacity(0);
    REQUIRE(buffer.empty());
}

TEST_CASE("Circular buffer copies are independent and moved-from buffers are reusable", "[circular-buffer]")
{
    CircularBuffer<int> source(2);
    for (int i : {1, 2, 3})
        source.push_back(i);
    auto copy = source;
    copy.front() = 10;
    REQUIRE(source.front() == 2);
    CircularBuffer<int> assigned;
    assigned = source;
    assigned.push_back(4);
    REQUIRE(source.back() == 3);

    CircularBuffer<int> moved(std::move(source));
    REQUIRE(std::vector<int>(moved.begin(), moved.end()) == std::vector<int>{2, 3});
    REQUIRE(source.empty());
    REQUIRE(source.capacity() == 0);
    source.push_back(1);
    source.set_capacity(1);
    source.push_back(5);
    REQUIRE(source.front() == 5);
    assigned = std::move(moved);
    REQUIRE(assigned.front() == 2);
    REQUIRE(moved.empty());
    REQUIRE(moved.capacity() == 0);
    auto& self = assigned;
    assigned = self;
    assigned = std::move(self);
    REQUIRE(assigned.back() == 3);
}

TEST_CASE("Circular buffer supports move-only non-default-constructible data", "[circular-buffer]")
{
    struct Value
    {
        std::unique_ptr<int> data;
        explicit Value(int i) : data(std::make_unique<int>(i)) {}
        Value(Value&&) noexcept = default;
        Value& operator=(Value&&) noexcept = default;
    };
    CircularBuffer<Value> buffer(2);
    buffer.push_back(Value(1));
    buffer.push_back(Value(2));
    buffer.push_back(Value(3));
    REQUIRE(*buffer.front().data == 2);
    buffer.set_capacity(4);
    REQUIRE(*buffer.back().data == 3);
    buffer.set_capacity(1);
    REQUIRE(*buffer.front().data == 2);
}

namespace {
struct ThrowingCopy
{
    inline static bool fail = false;
    int value;
    explicit ThrowingCopy(int value) : value(value) {}
    ThrowingCopy(const ThrowingCopy& other) : value(other.value)
    { if (fail) throw std::runtime_error("copy failed"); }
    ThrowingCopy& operator=(const ThrowingCopy& other)
    {
        if (fail) throw std::runtime_error("assignment failed");
        value = other.value;
        return *this;
    }
};
}

TEST_CASE("Circular buffer remains usable after element copy failures", "[circular-buffer]")
{
    CircularBuffer<ThrowingCopy> buffer(2);
    const ThrowingCopy value(1);
    buffer.push_back(value);
    ThrowingCopy::fail = true;
    CHECK_THROWS_AS(buffer.push_back(value), std::runtime_error);
    CHECK(buffer.size() == 1);
    CHECK_THROWS_AS(buffer.set_capacity(3), std::runtime_error);
    CHECK(buffer.capacity() == 2);
    CHECK(buffer.front().value == 1);
    ThrowingCopy::fail = false;
    buffer.push_back(value);
    ThrowingCopy::fail = true;
    CHECK_THROWS_AS(buffer.push_back(value), std::runtime_error);
    CHECK(buffer.size() == 2);
    CircularBuffer<ThrowingCopy> destination(1);
    CHECK_THROWS_AS(destination = buffer, std::runtime_error);
    CHECK(destination.empty());
    CHECK(destination.capacity() == 1);
    ThrowingCopy::fail = false;
    buffer.push_back(ThrowingCopy(2));
    REQUIRE(buffer.back().value == 2);
    buffer.clear();
    REQUIRE(buffer.empty());
}

TEST_CASE("Circular buffer releases removed and overwritten elements", "[circular-buffer]")
{
    CircularBuffer<std::shared_ptr<int>> buffer(2);
    const auto push = [&buffer](int value) {
        auto resource = std::make_shared<int>(value);
        std::weak_ptr<int> weak = resource;
        buffer.push_back(std::move(resource));
        return weak;
    };
    auto first = push(1);
    auto second = push(2);
    auto third = push(3);
    REQUIRE(first.expired());
    buffer.pop_front();
    REQUIRE(second.expired());
    buffer.clear();
    REQUIRE(third.expired());
    auto fourth = push(4);
    buffer.resize(0);
    REQUIRE(fourth.expired());
    auto fifth = push(5);
    buffer.set_capacity(0);
    REQUIRE(fifth.expired());
}

TEST_CASE("Circular buffer agrees with a deque model across mixed operations", "[circular-buffer]")
{
    CircularBuffer<int> buffer;
    std::deque<int> model;
    std::size_t capacity = 0;
    std::mt19937 random(417);
    for (int step = 0; step < 10000; ++step)
    {
        const auto operation = random() % 7;
        const auto count = random() % 17;
        INFO("step " << step << ", operation " << operation << ", count " << count);
        if (operation <= 1)
        {
            buffer.push_back(step);
            if (capacity != 0)
            {
                if (model.size() == capacity) model.pop_front();
                model.push_back(step);
            }
        }
        else if (operation == 2)
        {
            buffer.set_capacity(count);
            capacity = count;
            if (model.size() > capacity) model.resize(capacity);
        }
        else if (operation == 3)
        {
            buffer.resize(count, -1);
            capacity = (std::max)(capacity, static_cast<std::size_t>(count));
            model.resize(count, -1);
        }
        else if (operation == 4 && !model.empty())
        {
            buffer.pop_front();
            model.pop_front();
        }
        else if (operation == 5 && !model.empty())
        {
            buffer.pop_back();
            model.pop_back();
        }
        else if (operation == 6)
        {
            buffer.clear();
            model.clear();
        }
        REQUIRE(buffer.size() == model.size());
        REQUIRE(buffer.capacity() == capacity);
        REQUIRE(buffer.empty() == model.empty());
        REQUIRE(buffer.full() == (model.size() == capacity));
        REQUIRE(std::equal(buffer.begin(), buffer.end(), model.begin(), model.end()));
    }
}

namespace {
struct LifetimeValue
{
    inline static int live = 0;
    inline static int constructed = 0;
    inline static int destroyed = 0;
    inline static int copy_budget = -1;
    int value;

    explicit LifetimeValue(int value) : value(value) { ++live; ++constructed; }
    LifetimeValue(const LifetimeValue& other) : value(other.value)
    {
        if (copy_budget == 0) throw std::runtime_error("copy budget exhausted");
        if (copy_budget > 0) --copy_budget;
        ++live;
        ++constructed;
    }
    LifetimeValue(LifetimeValue&& other) noexcept(false) : value(other.value)
    { ++live; ++constructed; }
    LifetimeValue& operator=(const LifetimeValue&) = default;
    LifetimeValue& operator=(LifetimeValue&&) = default;
    ~LifetimeValue() { --live; ++destroyed; }
};

struct CopyBudgetReset
{
    ~CopyBudgetReset() { LifetimeValue::copy_budget = -1; }
};
}

TEST_CASE("Circular buffer constructs and destroys only live elements", "[circular-buffer][lifetime]")
{
    REQUIRE(LifetimeValue::live == 0);
    LifetimeValue::constructed = LifetimeValue::destroyed = 0;
    {
        CircularBuffer<LifetimeValue> buffer(100000);
        REQUIRE(LifetimeValue::constructed == 0);
        buffer.push_back(LifetimeValue(1));
        buffer.push_back(LifetimeValue(2));
        REQUIRE(LifetimeValue::live == 2);
        buffer.set_capacity(4);
        REQUIRE(LifetimeValue::live == 2);
        for (int i = 3; i <= 9; ++i)
            buffer.push_back(LifetimeValue(i));
        REQUIRE(LifetimeValue::live == 4);
        REQUIRE(buffer.front().value == 6);
        buffer.set_capacity(2);
        REQUIRE(LifetimeValue::live == 2);
        REQUIRE(buffer.front().value == 6);
        buffer.pop_front();
        REQUIRE(LifetimeValue::live == 1);
        buffer.clear();
        REQUIRE(LifetimeValue::live == 0);
        const auto constructions = LifetimeValue::constructed;
        buffer.set_capacity(100000);
        REQUIRE(LifetimeValue::constructed == constructions);
        buffer.push_back(LifetimeValue(10));
    }
    REQUIRE(LifetimeValue::live == 0);
    REQUIRE(LifetimeValue::constructed == LifetimeValue::destroyed);
}

TEST_CASE("Circular buffer rolls back partial copies across wrapped spans", "[circular-buffer][lifetime]")
{
    REQUIRE(LifetimeValue::live == 0);
    LifetimeValue::constructed = LifetimeValue::destroyed = 0;
    CopyBudgetReset reset;
    {
        CircularBuffer<LifetimeValue> source(5);
        for (int i = 0; i < 7; ++i)
            source.push_back(LifetimeValue(i));
        // The first physical span contains three live elements. Fail after that
        // span and one element in the second span have been copied successfully.
        LifetimeValue::copy_budget = 4;
        CHECK_THROWS_AS(CircularBuffer<LifetimeValue>(source), std::runtime_error);
        LifetimeValue::copy_budget = -1;
        REQUIRE(LifetimeValue::live == 5);
        REQUIRE(source.front().value == 2);
        REQUIRE(source.back().value == 6);

        CircularBuffer<LifetimeValue> destination(1);
        destination.push_back(LifetimeValue(99));
        LifetimeValue::copy_budget = 4;
        CHECK_THROWS_AS(destination = source, std::runtime_error);
        LifetimeValue::copy_budget = -1;
        REQUIRE(destination.front().value == 99);
        REQUIRE(destination.capacity() == 1);
        REQUIRE(LifetimeValue::live == 6);

        LifetimeValue::copy_budget = 4;
        CHECK_THROWS_AS(source.set_capacity(9), std::runtime_error);
        LifetimeValue::copy_budget = -1;
        REQUIRE(source.capacity() == 5);
        REQUIRE(source.size() == 5);
        REQUIRE(LifetimeValue::live == 6);
        for (std::size_t i = 0; i < source.size(); ++i)
            REQUIRE(source[i].value == int(i + 2));
        source.set_capacity(9);
        REQUIRE(source.size() == 5);
    }
    REQUIRE(LifetimeValue::live == 0);
    REQUIRE(LifetimeValue::constructed == LifetimeValue::destroyed);
}

TEST_CASE("Circular buffer supports over-aligned storage", "[circular-buffer][lifetime]")
{
    struct alignas(128) Value
    {
        int value;
        explicit Value(int value) : value(value) {}
    };
    CircularBuffer<Value> buffer(3);
    for (int i = 0; i < 8; ++i)
        buffer.push_back(Value(i));
    buffer.set_capacity(7);
    REQUIRE(buffer.front().value == 5);
    REQUIRE(buffer.back().value == 7);
    for (const auto& value : buffer)
        REQUIRE(reinterpret_cast<std::uintptr_t>(&value) % alignof(Value) == 0);
}

TEST_CASE("Cleared trivial-lifetime storage is not copied as live data", "[circular-buffer][lifetime]")
{
    struct Value
    {
        int value;
        explicit Value(int value) : value(value) {}
        Value(const Value&) { throw std::runtime_error("must not copy inactive data"); }
        Value(Value&&) noexcept = default;
        Value& operator=(Value&&) = default;
    };
    static_assert(std::is_trivially_destructible_v<Value>);
    CircularBuffer<Value> buffer(2);
    buffer.push_back(Value(1));
    buffer.clear();
    auto empty_copy = buffer;
    REQUIRE(empty_copy.empty());
    REQUIRE(empty_copy.capacity() == 2);
    buffer.set_capacity(4);
    buffer.push_back(Value(2));
    REQUIRE(buffer.front().value == 2);
}

TEST_CASE("Circular buffer rejects excessive capacity without changing data", "[circular-buffer]")
{
    CircularBuffer<int> buffer(2);
    buffer.push_back(42);
    REQUIRE_THROWS_AS(buffer.set_capacity((std::numeric_limits<std::size_t>::max)()), std::length_error);
    REQUIRE(buffer.capacity() == 2);
    REQUIRE(buffer.size() == 1);
    REQUIRE(buffer.front() == 42);
}
