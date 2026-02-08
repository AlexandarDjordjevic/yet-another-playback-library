# YAPL Performance Optimization Guide

This guide shows how to use the benchmarks to identify and fix performance bottlenecks.

## Quick Start

### 1. Establish Baseline (DEBUG build)

```bash
cmake --preset debug-linux
cmake --build build
./build/benchmarks/yapl_benchmarks --benchmark_out=baseline_debug.json --benchmark_out_format=json
```

### 2. Run with Optimizations

```bash
cmake --preset release-linux  # Or create one with -O3 -DNDEBUG
cmake --build build
./build/benchmarks/yapl_benchmarks --benchmark_out=baseline_release.json --benchmark_out_format=json
```

### 3. Identify Hotspots

Look for:
- **High latency** operations called frequently (> 1μs for hot paths)
- **Poor scalability** (sublinear scaling with threads)
- **Excessive allocations** (high bytes_per_second in allocation benchmarks)

## Current Performance Analysis

### Critical Findings (from baseline)

#### 1. Frame Allocation is the Bottleneck

```
BM_FrameAllocation_1080p:  22.7μs per allocation
BM_FramePool_vs_Individual: 9.4ns with pool reuse
```

**Impact**: 1080p @ 60fps requires 60 allocations/sec = 1.36ms/sec in allocation overhead
**Solution**: Implement object pooling (2,400x speedup!)

#### 2. Frame Copying is Expensive

```
BM_FrameCopy_1080p: 42.2μs per copy
```

**Impact**: Each frame copy wastes 42μs = 2.5ms at 60fps
**Solution**: Use move semantics or zero-copy techniques

#### 3. Clock Queries are Efficient

```
BM_MediaClock_GetTime: 16.8ns
BM_MediaClock_GetVideoTime: 17.5ns
```

**Status**: ✅ Good enough (60fps = 16.6ms budget, clock overhead < 0.001ms)
**Action**: No optimization needed

#### 4. Queue Performance is Excellent

```
BM_BlockingQueue_Push: 8-10ns
BM_BlockingQueue_Pop: 9-10ns
BM_BlockingQueue_SPSC: 15.5M ops/sec
```

**Status**: ✅ Meets requirements
**Action**: Monitor for regressions only

## Optimization Priorities

### Priority 1: Implement Frame Pooling

**Current**: 22.7μs allocation per 1080p frame
**Target**: < 100ns (pool lookup)
**Potential Gain**: 227x speedup

**Implementation**:
```cpp
class frame_pool {
    std::vector<std::shared_ptr<media_sample>> m_pool;
    std::mutex m_mutex;
    size_t m_next = 0;

public:
    frame_pool(size_t capacity, size_t frame_size) {
        m_pool.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            auto frame = std::make_shared<media_sample>();
            frame->data.resize(frame_size);
            m_pool.push_back(frame);
        }
    }

    std::shared_ptr<media_sample> acquire() {
        std::lock_guard lock(m_mutex);
        return m_pool[m_next++ % m_pool.size()];
    }
};
```

**Benchmark**:
```bash
# After implementing
./build/benchmarks/yapl_benchmarks --benchmark_filter=FramePool
# Compare against BM_FrameAllocation_1080p
```

### Priority 2: Eliminate Frame Copies

**Current**: 42.2μs per copy
**Target**: 0 (use std::move)

**Implementation**:
```cpp
// Before (copies frame data):
void decoder::decode(/* ... */, std::shared_ptr<media_sample> output) {
    // ... decode into temp buffer
    output->data = temp_buffer; // COPY!
}

// After (moves ownership):
void decoder::decode(/* ... */, std::shared_ptr<media_sample> output) {
    // ... decode into output->data directly
    // OR:
    output->data = std::move(temp_buffer); // MOVE!
}
```

**Verification**:
```bash
# Should see drastic reduction
./build/benchmarks/yapl_benchmarks --benchmark_filter=FrameCopy
```

### Priority 3: Reduce Allocations (if pooling not enough)

**Options**:
1. **Custom allocator**: jemalloc or tcmalloc
2. **Pre-sized vectors**: Reserve capacity upfront
3. **Stack allocation**: For small audio frames

**Benchmark custom allocator**:
```cpp
// In benchmark file, add:
template<typename T>
using pool_allocator = /* custom allocator */;

static void BM_CustomAllocator(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<uint8_t, pool_allocator<uint8_t>> data;
        data.resize(1920 * 1080 * 3 / 2);
        benchmark::DoNotOptimize(data);
    }
}
```

## Optimization Workflow

### Step 1: Measure (Before)

```bash
# Save baseline
git checkout main
cmake --preset release-linux && cmake --build build
./build/benchmarks/yapl_benchmarks --benchmark_out=before.json --benchmark_out_format=json
```

### Step 2: Optimize

Implement change (e.g., frame pooling)

### Step 3: Measure (After)

```bash
cmake --build build
./build/benchmarks/yapl_benchmarks --benchmark_out=after.json --benchmark_out_format=json
```

### Step 4: Compare

```bash
# Manual comparison
grep "FrameAllocation" before.json
grep "FrameAllocation" after.json

# Or use benchmark compare tool (if installed)
python3 tools/compare.py benchmarks before.json after.json
```

### Step 5: Verify Correctness

```bash
# Always run tests after optimization!
ctest --output-on-failure
```

### Step 6: Profile Real Workload

```bash
# Benchmarks show micro-performance, but profile real usage
perf record -g ./build/examples/yapl_player/yapl_player video.mp4
perf report
```

## Common Optimization Patterns

### Pattern 1: Lock-Free Queue (if queue becomes bottleneck)

```cpp
#include <boost/lockfree/spsc_queue.hpp>

// Replace blocking_queue for SPSC scenarios
boost::lockfree::spsc_queue<frame_ptr> m_frames{60};
```

**Benchmark**: Should see 2-3x improvement in `BM_BlockingQueue_SPSC`

### Pattern 2: Memory Alignment

```cpp
alignas(64) struct media_sample {  // Cache line alignment
    // ...
};
```

**Benchmark**: May improve `BM_SharedPtrRefCount` if false sharing exists

### Pattern 3: Batch Processing

```cpp
// Instead of: process 1 frame at a time
while (auto frame = queue.try_pop()) {
    process(frame);
}

// Do: process in batches
std::vector<frame_ptr> batch;
while (queue.try_pop_batch(batch, 10)) {
    for (auto& frame : batch) {
        process(frame);
    }
}
```

**Benchmark**: Should reduce synchronization overhead in `BM_BlockingQueue_MPMC`

## Regression Detection

### In CI Pipeline

```bash
#!/bin/bash
# ci_benchmark.sh

# Run benchmarks
./build/benchmarks/yapl_benchmarks \\
    --benchmark_filter="Critical" \\
    --benchmark_out=ci_results.json

# Compare against baseline (10% threshold)
python3 tools/compare.py \\
    --threshold=1.10 \\
    baseline.json ci_results.json

# Exit with error if regression detected
exit $?
```

### Watching for Regressions

Key metrics to monitor:
- `BM_BlockingQueue_Push` < 15ns
- `BM_MediaClock_GetTime` < 25ns
- `BM_FrameAllocation_1080p` < 100ns (after pooling)
- `BM_BlockingQueue_SPSC` > 10M ops/sec

## Expected Performance Targets

| Component | Current (DEBUG) | Target (Release + Opt) | Status |
|-----------|-----------------|------------------------|--------|
| Queue push | 8ns | < 5ns | ✅ Good |
| Queue pop | 10ns | < 5ns | ✅ Good |
| Clock query | 17ns | < 10ns | ✅ Good |
| Frame alloc | 23μs | < 100ns | ⚠️ **OPTIMIZE** |
| Frame copy | 42μs | 0 (eliminate) | ⚠️ **OPTIMIZE** |
| SPSC | 15M/s | > 50M/s | ⚠️ Improve |

## Additional Tools

### CPU Profiling

```bash
# Install perf
sudo apt-get install linux-tools-common

# Profile benchmark
perf record -g ./build/benchmarks/yapl_benchmarks --benchmark_filter=FrameAllocation
perf report

# Look for hotspots in:
# - malloc/free calls
# - memcpy
# - atomic operations
```

### Memory Profiling

```bash
# Valgrind
valgrind --tool=massif ./build/benchmarks/yapl_benchmarks
ms_print massif.out.* | less

# Look for:
# - Peak memory usage
# - Allocation patterns
# - Memory fragmentation
```

### Cache Analysis

```bash
# Cache misses
perf stat -e cache-misses,cache-references \\
    ./build/benchmarks/yapl_benchmarks --benchmark_filter=FrameCopy

# Look for:
# - High cache miss rate (> 5%)
# - False sharing between threads
```

## Success Criteria

After optimizations, verify:

1. ✅ **No test failures**: `ctest` passes 100%
2. ✅ **Benchmark improvements**: Targeted metrics improved
3. ✅ **No regressions**: Other benchmarks within 5% of baseline
4. ✅ **Real-world gain**: Measured with actual video playback
5. ✅ **Code quality**: No added complexity without justification

## References

- [Google Benchmark User Guide](https://github.com/google/benchmark/blob/main/docs/user_guide.md)
- [C++ Optimization Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#per-performance)
- [Perf Tutorial](https://perf.wiki.kernel.org/index.php/Tutorial)
