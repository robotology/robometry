/*
 * Copyright (C) 2026 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <robometry/CircularBuffer.h>
#include <robometry/Record.h>
#include <boost/circular_buffer.hpp>
#include <boost/version.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#include <intrin.h>
#define ROBOMETRY_BENCHMARK_NOINLINE __declspec(noinline)
#else
#define ROBOMETRY_BENCHMARK_NOINLINE __attribute__((noinline))
#endif

#ifdef ROBOMETRY_ALLOCATION_PROBE
namespace allocations {
bool enabled = false;
std::size_t count = 0;
std::size_t bytes = 0;

struct Scope
{
    Scope() { count = bytes = 0; enabled = true; }
    ~Scope() { enabled = false; }
};
}

// All measured types have fundamental alignment. These replacements belong only
// to the allocation executable, so timed payload allocations use the normal runtime.
void* operator new(std::size_t bytes)
{
    void* memory = std::malloc(bytes == 0 ? 1 : bytes);
    if (!memory)
        throw std::bad_alloc();
    if (allocations::enabled)
    {
        ++allocations::count;
        allocations::bytes += bytes;
    }
    return memory;
}
void* operator new[](std::size_t bytes) { return ::operator new(bytes); }
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t) noexcept { std::free(memory); }
#endif

namespace {
using Clock = std::chrono::steady_clock;
enum class Operation { overwrite, overwrite_move, fill_clear, iterate };
volatile double consumed = 0.0;

// Make each insertion observable without introducing a function call. Both
// containers use the same compiler barrier; there is no CPU fence on GCC/Clang.
template<class T>
void observe(const T& value)
{
#ifdef _MSC_VER
    _ReadWriteBarrier();
    (void)value;
#else
    asm volatile("" : : "g"(&value) : "memory");
#endif
}

double number(std::uint64_t value) { return static_cast<double>(value); }
double number(const robometry::Record& value) { return value.m_ts; }

template<class T>
using Inputs = std::array<T, 32>;

template<class T>
Inputs<T> inputs(bool vector_payload)
{
    Inputs<T> values{};
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if constexpr (std::is_same_v<T, robometry::Record>)
        {
            values[i].m_ts = static_cast<double>(i);
            if (vector_payload)
                values[i].m_datum = std::vector<double>(32, static_cast<double>(i));
            else
                values[i].m_datum = static_cast<double>(i);
        }
        else
        {
            values[i] = i;
        }
    }
    return values;
}

template<class Buffer, class T>
void fill(Buffer& buffer, const Inputs<T>& values)
{
    for (std::size_t i = 0; i < buffer.capacity(); ++i)
        buffer.push_back(values[i % values.size()]);
}

struct CapacityCase
{
    const char* name;
    std::size_t old_capacity;
    std::size_t old_size;
    std::size_t new_capacity;
    bool wrapped;

    std::size_t offset() const { return wrapped ? old_capacity / 3 + 1 : 0; }
    std::size_t retained() const { return (std::min)(old_size, new_capacity); }
};

std::vector<CapacityCase> capacityCases()
{
    std::vector<CapacityCase> cases;
    for (const auto n : {std::size_t{64}, std::size_t{1024}, std::size_t{10000}})
    {
        cases.push_back({"empty_grow", 0, 0, n, false});
        cases.push_back({"sparse_grow", n, n / 16, 2 * n, false});
        cases.push_back({"full_grow", n, n, 2 * n, false});
        cases.push_back({"wrapped_grow", n, n, 2 * n, true});
        cases.push_back({"full_shrink", n, n, n / 2, false});
        cases.push_back({"wrapped_shrink", n, n, n / 2, true});
        cases.push_back({"unchanged", n, n, n, true});
        cases.push_back({"release", n, n, 0, true});
    }
    cases.push_back({"empty_grow_large", 0, 0, 100000, false});
    cases.push_back({"sparse_grow_large", 64, 1, 100000, false});
    return cases;
}

template<class Buffer, class T>
void prepareCapacity(Buffer& buffer, const CapacityCase& config, const Inputs<T>& values)
{
    for (std::size_t i = 0; i < config.old_size + config.offset(); ++i)
        buffer.push_back(values[i % values.size()]);
    observe(buffer);
}

template<class T>
bool sameValue(const T& a, const T& b)
{
    if constexpr (std::is_same_v<T, robometry::Record>)
    {
        if (a.m_ts != b.m_ts || a.m_datum.type() != b.m_datum.type())
            return false;
        if (a.m_datum.type() == typeid(double))
            return std::any_cast<double>(a.m_datum) == std::any_cast<double>(b.m_datum);
        return std::any_cast<const std::vector<double>&>(a.m_datum)
               == std::any_cast<const std::vector<double>&>(b.m_datum);
    }
    else
    {
        return a == b;
    }
}

template<class Buffer, class T>
void validateCapacity(const Buffer& buffer, const CapacityCase& config,
                      const Inputs<T>& values, bool all_elements)
{
    if (buffer.capacity() != config.new_capacity || buffer.size() != config.retained())
        throw std::runtime_error(std::string("Unexpected capacity/size: ") + config.name);
    if (buffer.empty())
        return;
    if (!sameValue(buffer.front(), values[config.offset() % values.size()])
        || !sameValue(buffer.back(), values[(config.offset() + config.retained() - 1) % values.size()]))
        throw std::runtime_error(std::string("Unexpected retained endpoints: ") + config.name);
    if (all_elements)
    {
        for (std::size_t i = 0; i < buffer.size(); ++i)
            if (!sameValue(buffer[i], values[(config.offset() + i) % values.size()]))
                throw std::runtime_error(std::string("Unexpected retained element: ") + config.name);
    }
}

void capacityColumns(const char* payload, const CapacityCase& config)
{
    std::cout << payload << ',' << config.name << ',' << config.old_capacity << ','
              << config.old_size << ',' << config.new_capacity << ',' << config.retained() << ','
              << (config.wrapped ? 1 : 0) << ',';
}

template<class Buffer, class T>
ROBOMETRY_BENCHMARK_NOINLINE double run(Operation operation, Buffer& buffer,
                                      const Inputs<T>& values, std::size_t repetitions)
{
    double sum = 0.0;
    if (operation == Operation::overwrite)
    {
        for (std::size_t i = 0; i < repetitions; ++i)
        {
            buffer.push_back(values[i % values.size()]);
            observe(buffer);
        }
    }
    else if (operation == Operation::overwrite_move)
    {
        for (std::size_t i = 0; i < repetitions; ++i)
        {
            // Include construction of the record before moving it into storage,
            // as BufferManager does. Payload copies/allocations are timed too.
            T value(values[i % values.size()]);
            buffer.push_back(std::move(value));
            observe(buffer);
        }
    }
    else if (operation == Operation::fill_clear)
    {
        for (std::size_t repeat = 0; repeat < repetitions; ++repeat)
        {
            buffer.clear();
            for (std::size_t i = 0; i < buffer.capacity(); ++i)
            {
                buffer.push_back(values[i % values.size()]);
                observe(buffer);
            }
        }
    }
    else
    {
        for (std::size_t repeat = 0; repeat < repetitions; ++repeat)
        {
            observe(buffer);
            for (const auto& value : buffer)
                sum += number(value);
        }
    }
    return sum + number(buffer.front()) + number(buffer.back());
}

#ifndef ROBOMETRY_ALLOCATION_PROBE
template<class Buffer, class T>
double measure(Operation operation, Buffer& buffer, const Inputs<T>& values,
               std::size_t repetitions)
{
    const auto start = Clock::now();
    const double result = run(operation, buffer, values, repetitions);
    const auto end = Clock::now();
    consumed = result;
    return std::chrono::duration<double, std::nano>(end - start).count();
}

template<class Buffer, class T>
std::size_t calibrate(Operation operation, Buffer& buffer, const Inputs<T>& values,
                      double minimum_ns)
{
    const bool insertion = operation == Operation::overwrite || operation == Operation::overwrite_move;
    std::size_t repetitions = insertion ? 4096 : 1;
    while (measure(operation, buffer, values, repetitions) < minimum_ns)
    {
        if (repetitions > (std::size_t{1} << 30))
            throw std::runtime_error("Calibration exceeded its iteration limit");
        repetitions *= 2;
    }
    return repetitions;
}

double median(std::vector<double> samples)
{
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

// Each timed call gets a fresh source buffer. Setup and final destruction are
// outside the clock. Pooling bounds memory and amortizes the clock overhead;
// unlike a grow/shrink round trip, this measures only the requested transition.
template<class Buffer, class T>
double measureCapacity(const CapacityCase& config, const Inputs<T>& values,
                       bool vector_payload, std::size_t repetitions)
{
    if (config.old_capacity == config.new_capacity)
    {
        Buffer buffer(config.old_capacity);
        prepareCapacity(buffer, config, values);
        const auto start = Clock::now();
        for (std::size_t i = 0; i < repetitions; ++i)
        {
            buffer.set_capacity(config.new_capacity);
            observe(buffer);
        }
        const auto end = Clock::now();
        validateCapacity(buffer, config, values, false);
        return std::chrono::duration<double, std::nano>(end - start).count();
    }

    constexpr std::size_t memory_budget = 32 * 1024 * 1024;
    // Use a common conservative memory estimate to bound both container pools.
    const auto storage_bytes = (config.old_capacity + config.new_capacity) * sizeof(std::optional<T>);
    const auto payload_bytes = vector_payload ? config.old_size * (32 * sizeof(double) + 96) : 0;
    const auto bytes_per_buffer = (std::max)(std::size_t{1}, storage_bytes + payload_bytes);
    const auto pool_limit = (std::max)(std::size_t{1},
        (std::min)(std::size_t{4096}, memory_budget / bytes_per_buffer));
    double elapsed = 0.0;
    for (std::size_t done = 0; done < repetitions;)
    {
        const auto count = (std::min)(pool_limit, repetitions - done);
        std::vector<Buffer> buffers;
        buffers.reserve(count);
        for (std::size_t i = 0; i < count; ++i)
        {
            buffers.emplace_back(config.old_capacity);
            prepareCapacity(buffers.back(), config, values);
        }
        const auto start = Clock::now();
        for (auto& buffer : buffers)
        {
            buffer.set_capacity(config.new_capacity);
            observe(buffer);
        }
        const auto end = Clock::now();
        elapsed += std::chrono::duration<double, std::nano>(end - start).count();
        for (const auto& buffer : buffers)
            validateCapacity(buffer, config, values, false);
        done += count;
    }
    return elapsed;
}

template<class Buffer, class T>
std::size_t calibrateCapacity(const CapacityCase& config, const Inputs<T>& values,
                             bool vector_payload, double minimum_ns)
{
    // Also verify every retained payload before timing the cheaper endpoint checks.
    Buffer check(config.old_capacity);
    prepareCapacity(check, config, values);
    check.set_capacity(config.new_capacity);
    validateCapacity(check, config, values, true);
    std::size_t repetitions = config.old_capacity == config.new_capacity ? 4096 : 1;
    while (measureCapacity<Buffer>(config, values, vector_payload, repetitions) < minimum_ns)
    {
        if (repetitions > (std::size_t{1} << 26))
            throw std::runtime_error("Capacity calibration exceeded its iteration limit");
        repetitions *= 2;
    }
    return repetitions;
}

template<class T>
void capacityTimings(const char* payload, bool vector_payload, std::size_t sample_count,
                     double minimum_ns)
{
    const auto values = inputs<T>(vector_payload);
    for (const auto& config : capacityCases())
    {
        const auto boost_repetitions = calibrateCapacity<boost::circular_buffer<T>>(
            config, values, vector_payload, minimum_ns);
        const auto robometry_repetitions = calibrateCapacity<robometry::CircularBuffer<T>>(
            config, values, vector_payload, minimum_ns);
        std::vector<double> boost_samples, robometry_samples;
        for (std::size_t sample = 0; sample < sample_count; ++sample)
        {
            const auto boost_trial = [&] {
                boost_samples.push_back(measureCapacity<boost::circular_buffer<T>>(
                    config, values, vector_payload, boost_repetitions) / boost_repetitions);
            };
            const auto robometry_trial = [&] {
                robometry_samples.push_back(measureCapacity<robometry::CircularBuffer<T>>(
                    config, values, vector_payload, robometry_repetitions) / robometry_repetitions);
            };
            if (sample % 2 == 0) { boost_trial(); robometry_trial(); }
            else { robometry_trial(); boost_trial(); }
        }
        const auto boost_ns = median(boost_samples);
        const auto robometry_ns = median(robometry_samples);
        capacityColumns(payload, config);
        std::cout << boost_repetitions << ',' << robometry_repetitions << ',' << boost_ns << ','
                  << robometry_ns << ',' << robometry_ns / boost_ns << ','
                  << *std::min_element(boost_samples.begin(), boost_samples.end()) << ','
                  << *std::max_element(boost_samples.begin(), boost_samples.end()) << ','
                  << *std::min_element(robometry_samples.begin(), robometry_samples.end()) << ','
                  << *std::max_element(robometry_samples.begin(), robometry_samples.end()) << '\n';
    }
}

template<class T>
void timings(const char* payload, bool vector_payload, std::size_t sample_count,
             double minimum_ns)
{
    const auto values = inputs<T>(vector_payload);
    for (const auto capacity : {std::size_t{64}, std::size_t{1024}, std::size_t{10000}})
    {
        for (auto operation : {Operation::overwrite, Operation::overwrite_move,
                               Operation::fill_clear, Operation::iterate})
        {
            boost::circular_buffer<T> boost_buffer(capacity);
            robometry::CircularBuffer<T> robometry_buffer(capacity);
            fill(boost_buffer, values);
            fill(robometry_buffer, values);
            const auto boost_repetitions = calibrate(operation, boost_buffer, values, minimum_ns);
            const auto robometry_repetitions = calibrate(operation, robometry_buffer, values, minimum_ns);
            const bool insertion = operation == Operation::overwrite || operation == Operation::overwrite_move;
            const auto elements = insertion ? 1 : capacity;
            std::vector<double> boost_samples, robometry_samples;
            for (std::size_t sample = 0; sample < sample_count; ++sample)
            {
                const auto boost_trial = [&] {
                    boost_samples.push_back(measure(operation, boost_buffer, values, boost_repetitions)
                                            / (boost_repetitions * elements));
                };
                const auto robometry_trial = [&] {
                    robometry_samples.push_back(measure(operation, robometry_buffer, values, robometry_repetitions)
                                               / (robometry_repetitions * elements));
                };
                // Alternate order to reduce bias from CPU frequency changes.
                if (sample % 2 == 0) { boost_trial(); robometry_trial(); }
                else { robometry_trial(); boost_trial(); }
            }
            const auto boost_ns = median(boost_samples);
            const auto robometry_ns = median(robometry_samples);
            const char* name = operation == Operation::overwrite ? "overwrite_copy"
                               : operation == Operation::overwrite_move ? "overwrite_copy_then_move"
                               : operation == Operation::fill_clear ? "fill_clear" : "iterate";
            std::cout << payload << ',' << name << ',' << capacity << ','
                      << boost_repetitions * elements << ',' << robometry_repetitions * elements << ','
                      << boost_ns << ',' << robometry_ns << ',' << robometry_ns / boost_ns << ','
                      << *std::min_element(boost_samples.begin(), boost_samples.end()) << ','
                      << *std::max_element(boost_samples.begin(), boost_samples.end()) << ','
                      << *std::min_element(robometry_samples.begin(), robometry_samples.end()) << ','
                      << *std::max_element(robometry_samples.begin(), robometry_samples.end()) << '\n';
        }
    }
}
#else
template<bool NothrowMove>
struct TransferValue
{
    inline static std::size_t copies = 0;
    inline static std::size_t moves = 0;
    std::uint64_t value{0};
    TransferValue() = default;
    TransferValue(std::uint64_t value) : value(value) {}
    TransferValue(const TransferValue& other) : value(other.value) { ++copies; }
    TransferValue(TransferValue&& other) noexcept(NothrowMove) : value(other.value) { ++moves; }
    TransferValue& operator=(const TransferValue&) = default;
    TransferValue& operator=(TransferValue&&) = default;
    bool operator==(const TransferValue& other) const { return value == other.value; }
};

template<class T> struct IsTransferValue : std::false_type {};
template<bool N> struct IsTransferValue<TransferValue<N>> : std::true_type {};

template<class Buffer, class T>
void capacityProbe(const char* container, const char* payload, const CapacityCase& config,
                   const Inputs<T>& values)
{
    Buffer buffer(config.old_capacity);
    prepareCapacity(buffer, config, values);
    if constexpr (IsTransferValue<T>::value)
        T::copies = T::moves = 0;
    {
        allocations::Scope tracking;
        buffer.set_capacity(config.new_capacity);
        observe(buffer);
    }
    const auto count = allocations::count;
    const auto bytes = allocations::bytes;
    const auto changed = config.old_capacity != config.new_capacity;
    const auto expected_allocations = changed && config.new_capacity != 0 ? 1 : 0;
    if (count != static_cast<std::size_t>(expected_allocations))
        throw std::runtime_error(std::string("Unexpected set_capacity allocation count: ") + config.name);
    validateCapacity(buffer, config, values, true);
    capacityColumns(payload, config);
    std::cout << container << ',' << count << ',' << bytes << ',';
    if constexpr (IsTransferValue<T>::value)
    {
        const auto expected_transfers = changed ? config.retained() : 0;
        const bool moved = std::is_nothrow_move_constructible_v<T>;
        if (T::copies != (moved ? 0 : expected_transfers)
            || T::moves != (moved ? expected_transfers : 0))
            throw std::runtime_error(std::string("Unexpected element transfers: ") + config.name);
        std::cout << T::copies << ',' << T::moves;
    }
    else
    {
        std::cout << ','; // Counts are available only for explicitly instrumented types.
    }
    std::cout << '\n';
}

template<class T>
void capacityProbes(const char* payload, bool vector_payload)
{
    const auto values = inputs<T>(vector_payload);
    for (const auto& config : capacityCases())
    {
        if constexpr (IsTransferValue<T>::value)
            if (config.old_capacity != 1024) continue;
        capacityProbe<boost::circular_buffer<T>>("boost", payload, config, values);
        capacityProbe<robometry::CircularBuffer<T>>("robometry", payload, config, values);
    }
}

template<class Buffer, class T>
void probe(const char* container, const char* payload, const Inputs<T>& values, std::size_t capacity)
{
    constexpr std::size_t operations = 10000;
    std::optional<Buffer> storage;
    {
        allocations::Scope tracking;
        storage.emplace(capacity);
    }
    const auto print = [&](const char* phase, std::size_t count) {
        std::cout << payload << ',' << capacity << ',' << container << ',' << phase << ','
                  << count << ',' << allocations::count << ',' << allocations::bytes << '\n';
    };
    print("construct", 1);
    fill(*storage, values);
    {
        allocations::Scope tracking;
        for (std::size_t i = 0; i < operations; ++i)
        {
            storage->push_back(values[i % values.size()]);
            observe(*storage);
        }
    }
    print("overwrite_copy", operations);
    if constexpr (!std::is_same_v<T, robometry::Record>)
    {
        if (allocations::count != 0)
            throw std::runtime_error("Unexpected scalar insertion allocation");
    }
    {
        allocations::Scope tracking;
        run(Operation::fill_clear, *storage, values, 10);
    }
    print("fill_clear", 10 * capacity);

    // Build payloads before counting to separate container work from payload
    // allocation. Moving these records into a full buffer must not allocate.
    std::vector<T> prepared;
    prepared.reserve(operations);
    for (std::size_t i = 0; i < operations; ++i)
        prepared.push_back(values[i % values.size()]);
    {
        allocations::Scope tracking;
        for (auto& value : prepared)
        {
            storage->push_back(std::move(value));
            observe(*storage);
        }
    }
    print("overwrite_move_prebuilt", operations);
    if (allocations::count != 0)
        throw std::runtime_error("Moving prepared payloads unexpectedly allocated");
}

template<class T>
void probes(const char* payload, bool vector_payload)
{
    const auto values = inputs<T>(vector_payload);
    for (const auto capacity : {std::size_t{64}, std::size_t{1024}, std::size_t{10000}})
    {
        probe<boost::circular_buffer<T>>("boost", payload, values, capacity);
        probe<robometry::CircularBuffer<T>>("robometry", payload, values, capacity);
    }
}
#endif
} // namespace

int main(int argc, char** argv)
{
#ifndef NDEBUG
    std::cerr << "Build this benchmark in Release mode (NDEBUG required).\n";
    return 1;
#endif
    try
    {
        static_assert(std::is_nothrow_move_constructible_v<robometry::Record>,
                      "Record must select the move path in move_if_noexcept");
        const bool capacity_mode = argc > 1 && std::string(argv[1]) == "--capacity";
        std::cerr << "compiler=" << ROBOMETRY_BENCHMARK_COMPILER << " boost=" << BOOST_LIB_VERSION
                  << " sizeof(uint64_t)=" << sizeof(std::uint64_t)
                  << " sizeof(Record)=" << sizeof(robometry::Record)
                  << " sizeof(robometry_container)=" << sizeof(robometry::CircularBuffer<robometry::Record>)
                  << " sizeof(boost_container)=" << sizeof(boost::circular_buffer<robometry::Record>) << '\n';
        std::cout << std::fixed << std::setprecision(3);
#ifdef ROBOMETRY_ALLOCATION_PROBE
        if (capacity_mode)
        {
            std::cout << "payload,case,old_capacity,old_size,new_capacity,retained,wrapped,container,"
                         "allocations,allocated_bytes,element_copies,element_moves\n";
            capacityProbes<std::uint64_t>("uint64", false);
            capacityProbes<robometry::Record>("record_double", false);
            capacityProbes<robometry::Record>("record_vector32", true);
            capacityProbes<TransferValue<true>>("tracked_nothrow_move", false);
            capacityProbes<TransferValue<false>>("tracked_throwing_move", false);
            return 0;
        }
        std::cout << "payload,capacity,container,phase,operations,allocations,allocated_bytes\n";
        probes<std::uint64_t>("uint64", false);
        probes<robometry::Record>("record_double", false);
        probes<robometry::Record>("record_vector32", true);
#else
        const int first = capacity_mode ? 2 : 1;
        const auto sample_count = argc > first ? std::stoul(argv[first]) : 9;
        const auto minimum_ms = argc > first + 1 ? std::stod(argv[first + 1]) : 20.0;
        if (sample_count < 3 || sample_count % 2 == 0 || !(minimum_ms > 0.0) || !std::isfinite(minimum_ms))
            throw std::invalid_argument("Usage: circular_buffer_benchmark [--capacity] [odd samples >= 3] [milliseconds > 0]");
        std::cerr << "samples=" << sample_count << " minimum_sample_ms=" << minimum_ms << '\n';
        if (capacity_mode)
        {
            std::cout << "payload,case,old_capacity,old_size,new_capacity,retained,wrapped,"
                         "boost_operations,robometry_operations,boost_median_ns,robometry_median_ns,"
                         "ratio_robometry_boost,boost_min_ns,boost_max_ns,robometry_min_ns,robometry_max_ns\n";
            capacityTimings<std::uint64_t>("uint64", false, sample_count, minimum_ms * 1e6);
            capacityTimings<robometry::Record>("record_double", false, sample_count, minimum_ms * 1e6);
            capacityTimings<robometry::Record>("record_vector32", true, sample_count, minimum_ms * 1e6);
            return 0;
        }
        std::cout << "payload,operation,capacity,boost_operations,robometry_operations,boost_median_ns,"
                     "robometry_median_ns,ratio_robometry_boost,boost_min_ns,boost_max_ns,"
                     "robometry_min_ns,robometry_max_ns\n";
        timings<std::uint64_t>("uint64", false, sample_count, minimum_ms * 1e6);
        timings<robometry::Record>("record_double", false, sample_count, minimum_ms * 1e6);
        timings<robometry::Record>("record_vector32", true, sample_count, minimum_ms * 1e6);
#endif
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
