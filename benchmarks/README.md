# Circular-buffer benchmarks

This compares `boost::circular_buffer<T>` with `robometry::CircularBuffer<T>`,
which uses uninitialized storage allocated with `std::allocator<T>`. Boost is
required by these standalone tools, not by the robometry library or its normal
unit tests.

## Benchmark modes

- **Steady state** (`circular_buffer_benchmark`): insertion, clear/refill and
  iteration at fixed capacity. Times are **nanoseconds per inserted/visited
  element**. Clear/refill includes clearing amortized over inserted elements.
- **Capacity changes** (`circular_buffer_benchmark --capacity`): one complete
  `set_capacity()` call. Tables use **microseconds per call**, while CSVs retain
  nanoseconds per call. This is a different operation with different costs.
- A ratio is **Robometry time / Boost time for the same row**. It is not a
  global score. Tables at capacity 1024 are representative, not maximum ratios
  over all cases. An O(1) no-op can also have a large ratio at sub-nanosecond or
  few-nanosecond absolute times; those controls are reported separately.
## Storage and element lifetime

An allocation reserves `capacity * sizeof(T)` bytes without constructing empty
slots. The head and live-element count determine which objects exist, so no
per-slot occupancy flag is needed. `set_capacity` transfers the retained prefix
using `uninitialized_move_n` or `uninitialized_copy_n` across at most two physical
spans, with copy fallback when needed for exception safety. Destruction visits
only live objects; clearing trivially destructible values is O(1).

The container requires C++17. Copies own independent storage;
copies of the `Buffer` wrapper continue to share their storage. Payload allocations
inside `std::any`/`std::vector` remain separate from container allocation.

## Conditions

Recorded on 2026-11-05: Ubuntu 24.04.4 / WSL2, Intel Core Ultra 9 185H,
CPU affinity 2, Boost 1.83, GCC 13.3 and GCC 11.5, C++17, `-O3 -DNDEBUG`, no LTO
or sanitizers in performance runs. GCC 11 runs on the same Ubuntu 24.04 host;
this is not a native Ubuntu 22.04 or Windows measurement. Each case uses nine
measured batches calibrated to a 20 ms target, alternating backend order.
The CSVs include minimum and maximum batch averages. These are local throughput
measurements, not individual-call worst-case latencies or an end-to-end logger
benchmark. Allocator state, cache/first-touch effects, CPU frequency and host
load affect results, especially small differences. No clock overhead is subtracted.

## Current steady-state results, capacity 1024

`overwrite_copy` copies a prepared value. `overwrite_copy_then_move` copies an
input to prepare a new record and then moves it into the full buffer; preparation
and payload allocations are timed. `iterate` sums values or timestamps only.
Inputs cycle through 32 prepared values; capacities are 64, 1024 and 10000.

### GCC 13.3 — ns per element

| Value | Operation | Boost ns/element | Robometry ns/element | Robometry / Boost |
|---|---|---:|---:|---:|
| `uint64_t` | `overwrite_copy` | 2.120 | 0.633 | 0.30x |
| `uint64_t` | `overwrite_copy_then_move` | 2.217 | 0.688 | 0.31x |
| `uint64_t` | `fill_clear` | 0.886 | 0.802 | 0.91x |
| `uint64_t` | `iterate` | 0.509 | 0.568 | 1.12x |
| Record + double | `overwrite_copy` | 7.089 | 6.665 | 0.94x |
| Record + double | `overwrite_copy_then_move` | 5.229 | 6.649 | 1.27x |
| Record + double | `fill_clear` | 4.348 | 4.461 | 1.03x |
| Record + double | `iterate` | 0.537 | 0.500 | 0.93x |
| Record + vector of 32 doubles | `overwrite_copy` | 27.889 | 28.669 | 1.03x |
| Record + vector of 32 doubles | `overwrite_copy_then_move` | 27.399 | 29.932 | 1.09x |
| Record + vector of 32 doubles | `fill_clear` | 33.248 | 55.103 | 1.66x |
| Record + vector of 32 doubles | `iterate` | 0.510 | 0.559 | 1.10x |

### GCC 11.5 — ns per element

| Value | Operation | Boost ns/element | Robometry ns/element | Robometry / Boost |
|---|---|---:|---:|---:|
| `uint64_t` | `overwrite_copy` | 2.403 | 0.728 | 0.30x |
| `uint64_t` | `overwrite_copy_then_move` | 2.601 | 0.739 | 0.28x |
| `uint64_t` | `fill_clear` | 0.972 | 0.968 | 0.99x |
| `uint64_t` | `iterate` | 0.616 | 0.502 | 0.81x |
| Record + double | `overwrite_copy` | 6.997 | 7.869 | 1.12x |
| Record + double | `overwrite_copy_then_move` | 6.224 | 7.515 | 1.21x |
| Record + double | `fill_clear` | 3.982 | 3.969 | 1.00x |
| Record + double | `iterate` | 0.575 | 0.512 | 0.89x |
| Record + vector of 32 doubles | `overwrite_copy` | 23.538 | 27.526 | 1.17x |
| Record + vector of 32 doubles | `overwrite_copy_then_move` | 22.662 | 23.397 | 1.03x |
| Record + vector of 32 doubles | `fill_clear` | 32.666 | 32.768 | 1.00x |
| Record + vector of 32 doubles | `iterate` | 0.527 | 0.579 | 1.10x |

Largest ratios among all 36 measured steady-state medians (not only capacity 1024):

- GCC 13: 1.66x for `record_vector32` / `fill_clear` at capacity 1024: 33.248 ns for Boost vs 55.103 ns for the raw-storage buffer.
- GCC 11: 1.21x for `record_double` / `overwrite_copy_then_move` at capacity 1024: 6.224 ns for Boost vs 7.515 ns for the raw-storage buffer.

There is no claim that the new container wins every case. All capacities and
batch ranges are in [the GCC 13 CSV](results/raw-gcc13-steady.csv) and
[the GCC 11 CSV](results/raw-gcc11-steady.csv).

## Current capacity-change results — us per call

Source preparation is outside the timer. Allocation, retained-element transfers
and cleanup of source storage are timed; final destruction of the result is not.
See [CAPACITY.md](CAPACITY.md) for all cases, allocation checks and methodology.

| Case | Capacity change | Boost us/call | Robometry us/call | Robometry / Boost |
|---|---|---:|---:|---:|
| Empty, scalar records | 0 → 1024 | 0.853 | 0.809 | 0.95x |
| Grow, 64 scalar records | 1024 → 2048 | 0.848 | 0.935 | 1.10x |
| Grow, 1024 scalar records | 1024 → 2048 | 7.017 | 6.047 | 0.86x |
| Grow, 1024 scalar records, wrapped | 1024 → 2048 | 7.341 | 6.637 | 0.90x |
| Shrink, 1024 scalar records | 1024 → 512 | 2.915 | 2.937 | 1.01x |
| Grow, 1024 vector records | 1024 → 2048 | 4.853 | 4.457 | 0.92x |
| Shrink, 1024 vector records | 1024 → 512 | 18.146 | 17.183 | 0.95x |
| Empty, large Record buffer | 0 → 100000 | 1.798 | 1.833 | 1.02x |

Largest ratios among the 78 capacity cases, excluding unchanged-capacity controls:

- GCC 13: 1.13x for `record_double` / `release` at capacity 10000: 32628.847 ns for Boost vs 36757.758 ns for the raw-storage buffer.
- GCC 11: 1.15x for `record_double` / `sparse_grow` at capacity 1024: 825.224 ns for Boost vs 944.482 ns for the raw-storage buffer.

Among the unchanged-capacity controls, the largest GCC 13 ratio is
2.70x for `record_double` / `unchanged` at capacity 64: 0.527 ns for Boost vs 1.422 ns for the raw-storage buffer. An absolute difference at that scale should
not be confused with transferring or allocating a large buffer.

## Allocation and storage audit

On the measured platform:

| Type | Boost bytes/slot | Robometry bytes/slot |
|---|---:|---:|
| `uint64_t` | 8 | 8 |
| `Record` | 24 | 24 |

At capacity 1024, both robometry and Boost request **24576 bytes** for Record
storage. These are element-storage allocation sizes, excluding allocator overhead
and heap-owned payloads, not process RSS. The container object is 32 bytes for
robometry and 40 bytes for Boost on this platform.

Both containers allocate once on construction at nonzero capacity.
Scalar/Record-with-double insertions allocate zero times. Vector-record copy
insertion allocates for the payload in both implementations; moving prebuilt
records allocates zero times. Capacity changes to a different nonzero capacity
allocate one destination block; no-op changes and release to zero allocate none.

- [GCC 13 storage/insertion allocation audit](results/raw-gcc13-allocations.csv)
- [GCC 11 storage/insertion allocation audit](results/raw-gcc11-allocations.csv)
- [GCC 13 capacity allocation/transfer audit](results/raw-gcc13-capacity-allocations.csv)
- [GCC 11 capacity allocation/transfer audit](results/raw-gcc11-capacity-allocations.csv)

## Reproduce

```sh
cmake -S benchmarks -B build-buffer-benchmark-raw -DCMAKE_BUILD_TYPE=Release
cmake --build build-buffer-benchmark-raw --config Release --parallel 2
./build-buffer-benchmark-raw/circular_buffer_benchmark 9 20 > steady.csv
./build-buffer-benchmark-raw/circular_buffer_benchmark --capacity 9 20 > capacity.csv
./build-buffer-benchmark-raw/circular_buffer_allocations > allocations.csv
./build-buffer-benchmark-raw/circular_buffer_allocations --capacity > capacity-allocations.csv
ctest --test-dir build-buffer-benchmark-raw --output-on-failure
```

Boost and matio-cpp development files are required. For custom installations,
pass `-DCMAKE_PREFIX_PATH=/path/to/dependencies`. On Linux, measurements above use
`taskset -c 2` before each timing command. Visual Studio executables are in the
`Release` directory. The CLI accepts an odd sample count of at least 3 and a
positive calibration target in milliseconds. Allocation interception is compiled
only into the allocation executable, never the timing executable.

## Validation

The core suite has 22 passing tests on GCC 11/GCC 13 and ASan/UBSan/LSan, including
five tests for live-object counts, partial-copy rollback across wrapped spans,
over-alignment, trivial-lifetime reuse and excessive capacity. Benchmark probes
validate retained payloads/order and allocation/copy/move counts. MSVC remains
covered by the repository CI configuration; it was not run locally.
