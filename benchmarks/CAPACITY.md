# Capacity-change benchmarks

This compares `set_capacity()` in `robometry::CircularBuffer<T>` and
`boost::circular_buffer<T>`. It measures a complete capacity change, separately
from the insertion and iteration workloads in [README.md](README.md).

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

## Storage and transfers

Robometry allocates uninitialized storage with `std::allocator<T>`. It
constructs/transfers only retained live elements, using standard uninitialized
move/copy algorithms across at most two contiguous spans. It destroys only live
objects before releasing source storage. The head and element count identify
active elements; storage uses `sizeof(T)` bytes per slot.

A transfer uses move when it is noexcept (or copying is unavailable), otherwise
copy. A completed span is tracked by the destination's live count; the standard
algorithms clean up a failed partial span. Tests exercise failure after the
first wrapped span has already been copied successfully.

## Robometry versus Boost

### GCC 13.3 — us per capacity change

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

### GCC 11.5 — us per capacity change

| Case | Capacity change | Boost us/call | Robometry us/call | Robometry / Boost |
|---|---|---:|---:|---:|
| Empty, scalar records | 0 → 1024 | 0.866 | 0.907 | 1.05x |
| Grow, 64 scalar records | 1024 → 2048 | 0.825 | 0.944 | 1.15x |
| Grow, 1024 scalar records | 1024 → 2048 | 6.966 | 6.550 | 0.94x |
| Grow, 1024 scalar records, wrapped | 1024 → 2048 | 7.088 | 6.916 | 0.98x |
| Shrink, 1024 scalar records | 1024 → 512 | 3.066 | 2.767 | 0.90x |
| Grow, 1024 vector records | 1024 → 2048 | 5.415 | 5.211 | 0.96x |
| Shrink, 1024 vector records | 1024 → 512 | 16.777 | 17.025 | 1.01x |
| Empty, large Record buffer | 0 → 100000 | 1.758 | 1.818 | 1.03x |

Largest measured median ratios for actual changes (all 78 cases):

- GCC 13: 1.13x for `record_double` / `release` at capacity 10000: 32628.847 ns for Boost vs 36757.758 ns for the raw-storage buffer.
- GCC 11: 1.15x for `record_double` / `sparse_grow` at capacity 1024: 825.224 ns for Boost vs 944.482 ns for the raw-storage buffer.

The unchanged-capacity control makes no allocations or transfers. Within those
controls, the largest GCC 13 ratio is 2.70x for `record_double` / `unchanged` at capacity 64: 0.527 ns for Boost vs 1.422 ns for the raw-storage buffer. Its absolute
nanosecond cost is reported separately to avoid presenting it as a large-buffer
reallocation cost. No universal or worst-case latency guarantee follows from
these batch averages.

## Method and checks

There are 26 cases for each of three payloads (uint64, Record holding double,
Record holding a vector of 32 doubles): **78 timing cases per compiler**.
Capacities 64/1024/10000 cover empty, sparse (1/16 occupancy), full and wrapped
full growth; full and wrapped shrink; unchanged capacity; and release to zero.
Additional cases grow an empty or one-element buffer to 100000 slots.

Source buffers and payloads are constructed outside the clock. Each changed
capacity call gets a fresh source buffer; growth/shrink are not combined into a
round trip. The timer includes replacement allocation, element transfers and source
storage cleanup. Result destruction is outside. Pools use a common conservative
32 MiB memory estimate for both containers, with a maximum of 4096 buffers. Multiple timed chunks accumulate the batch duration.
A common compiler barrier prevents elision. Every retained payload is validated
before calibration, with size/capacity/endpoints checked after every timed call.

Separate allocation probes audit all real cases and two instrumented types.
For the current Record workloads, there is one allocation for a changed nonzero
capacity and zero extra payload allocations. Growing 1024 live vector records to
capacity 2048 requests 49152 bytes in both robometry and Boost. The noexcept-movable probe type
reports 1024 moves and no copies; a potentially-throwing-move probe reports 1024
copies and no moves. A no-op or release to zero allocates nothing. Tests validate
all retained data and both physical source spans, including rollback on failure.

## Reproduce

Build the tools using the commands in [README.md](README.md), then run:

```sh
taskset -c 2 ./build-buffer-benchmark-raw/circular_buffer_benchmark --capacity 9 20 > capacity.csv
./build-buffer-benchmark-raw/circular_buffer_allocations --capacity > capacity-allocations.csv
ctest --test-dir build-buffer-benchmark-raw --output-on-failure
```

CPU affinity via `taskset` is optional and Linux-specific. Performance runs do not
use sanitizers; sanitizer checks exercise semantics only. Allocation sizes exclude
allocator metadata, container/control objects and separately allocated payloads.

## Raw data

- [Current GCC 13 timing](results/raw-gcc13-capacity.csv)
- [Current GCC 11 timing](results/raw-gcc11-capacity.csv)
- [Current GCC 13 allocation/transfer audit](results/raw-gcc13-capacity-allocations.csv)
- [Current GCC 11 allocation/transfer audit](results/raw-gcc11-capacity-allocations.csv)
