# YAPL Performance Benchmarks

Comprehensive benchmarks for critical performance paths in YAPL.

## Building

```bash
cmake --preset debug-linux
cmake --build build
```

## Running Benchmarks

Run all benchmarks:
```bash
./build/benchmarks/yapl_benchmarks
```

Run specific benchmark:
```bash
./build/benchmarks/yapl_benchmarks --benchmark_filter=BlockingQueue
```

Run with specific iterations:
```bash
./build/benchmarks/yapl_benchmarks --benchmark_min_time=5.0
```

Output to JSON:
```bash
./build/benchmarks/yapl_benchmarks --benchmark_out=results.json --benchmark_out_format=json
```

## Benchmark Categories

### 1. Blocking Queue (`blocking_queue_bench.cpp`)

**Critical Path**: Queue operations occur on every frame (60+ times/second)

Benchmarks:
- `BM_BlockingQueue_Push` - Single-threaded push throughput
- `BM_BlockingQueue_Pop` - Single-threaded pop throughput
- `BM_BlockingQueue_SharedPtr` - Realistic frame allocation + queue
- `BM_BlockingQueue_SPSC` - Single producer, single consumer
- `BM_BlockingQueue_MPMC` - Multiple producers, multiple consumers
- `BM_BlockingQueue_Stats` - Queue stats query overhead

**Why This Matters**:
- Queues are the primary inter-thread communication mechanism
- Poor queue performance directly impacts frame drops
- Contention in MPMC scenarios affects scalability

### 2. Media Clock (`media_clock_bench.cpp`)

**Critical Path**: Time queries on every frame render

Benchmarks:
- `BM_MediaClock_GetTime` - Raw time query (audio renderer)
- `BM_MediaClock_GetVideoTime` - Video time with latency compensation
- `BM_MediaClock_ConcurrentReads` - Multi-threaded read contention
- `BM_MediaClock_PauseResume` - Pause/resume overhead
- `BM_MediaClock_SetAudioLatency` - Latency update (every audio frame)
- `BM_MediaClock_IsStarted` - State check overhead
- `BM_MediaClock_GetTimeWhilePaused` - Paused time calculation

**Why This Matters**:
- Called 60+ times/sec for video, 44100/sec for audio samples
- Any overhead here multiplies across all frames
- Atomic operations must be lock-free for performance

### 3. Memory Allocation (`memory_allocation_bench.cpp`)

**Critical Path**: Frame allocation on every decode

Benchmarks:
- `BM_FrameAllocation_1080p/720p/4K` - Video frame allocation
- `BM_AudioFrameAllocation` - Audio frame allocation
- `BM_FrameCopy_1080p` - Frame data copying
- `BM_SharedPtrRefCount` - Reference counting overhead
- `BM_FramePool_vs_Individual` - Pool reuse vs malloc
- `BM_VectorReserve_vs_Resize` - Allocation strategies
- `BM_MediaSample_StructSize` - Struct allocation overhead

**Why This Matters**:
- 1080p YUV frame = 3MB allocation per frame
- At 60fps, that's 180MB/sec allocation rate
- Memory fragmentation can cause frame drops
- Pool reuse can dramatically reduce allocator pressure

## Baseline Metrics (Target)

Based on typical playback requirements:

| Metric | Target | Rationale |
|--------|--------|-----------|
| Queue push/pop | < 100ns | 60fps = 16.6ms/frame budget |
| Clock query | < 50ns | Called multiple times per frame |
| Frame allocation (1080p) | < 500μs | Decoder runs async, but adds latency |
| SPSC throughput | > 10M ops/sec | Video + audio streams |

## Performance Optimization Guide

### Before Optimizing

1. **Establish Baseline**: Run benchmarks and save results
   ```bash
   ./build/benchmarks/yapl_benchmarks --benchmark_out=baseline.json
   ```

2. **Identify Hotspots**: Focus on benchmarks showing:
   - High latency (> 1μs for frequent operations)
   - Poor scalability (threads > 1)
   - Excessive allocations (high bytes/iteration)

3. **Profile Real Workload**: Benchmarks complement, not replace profiling

### After Optimizing

1. **Compare Results**:
   ```bash
   # Run optimized version
   ./build/benchmarks/yapl_benchmarks --benchmark_out=optimized.json

   # Compare (requires benchmark tools)
   compare.py baseline.json optimized.json
   ```

2. **Verify No Regressions**: Check all benchmarks, not just optimized ones

3. **Run Tests**: Ensure correctness with `ctest`

## Common Optimization Strategies

### Queue Performance
- **Lock-free queues**: Consider boost::lockfree for SPSC
- **Batch operations**: Reduce synchronization overhead
- **Size tuning**: Benchmark different queue sizes

### Clock Performance
- **Cache time values**: If precision allows, cache for 1ms
- **Reduce atomic operations**: Use relaxed ordering where safe
- **Coarse-grained timing**: Use lower resolution clocks

### Memory Performance
- **Object pooling**: Reuse frame buffers
- **Custom allocators**: jemalloc, tcmalloc for better concurrency
- **Reserve capacity**: Pre-allocate vectors to avoid realloc
- **Align allocations**: Cache-line alignment for frequently accessed data

## Interpreting Results

### Throughput (items/sec)
Higher is better. Compare against frame rate requirements.

### Latency (Time)
Lower is better. Watch for outliers in min/max/stddev.

### CPU Time vs Real Time
- `CPU time > Real time`: CPU-bound operation
- `CPU time < Real time`: I/O or contention

### Scaling with Threads
- Linear: Good parallelization
- Sublinear: Contention issues
- Negative: Serious synchronization problems

## Continuous Benchmarking

Add to CI:
```bash
# Detect regressions > 10%
./build/benchmarks/yapl_benchmarks --benchmark_filter=Critical \
    --benchmark_out=ci_results.json
compare_bench.py baseline.json ci_results.json --threshold=1.1
```
