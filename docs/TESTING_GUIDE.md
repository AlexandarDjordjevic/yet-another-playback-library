# YAPL Testing and Quality Assurance Guide

Comprehensive guide for running tests, measuring code coverage, and detecting bugs with sanitizers.

## Quick Reference

```bash
# Unit tests (debug build)
cmake --preset debug-linux && cmake --build build
ctest --test-dir build --output-on-failure

# Code coverage
cmake --preset coverage-linux && cmake --build build-coverage
ctest --test-dir build-coverage
./scripts/generate_coverage.sh

# AddressSanitizer (memory errors)
cmake --preset asan-linux && cmake --build build-asan
ctest --test-dir build-asan

# ThreadSanitizer (race conditions)
cmake --preset tsan-linux && cmake --build build-tsan
TSAN_OPTIONS="second_deadlock_stack=1" ctest --test-dir build-tsan

# UndefinedBehaviorSanitizer (undefined behavior)
cmake --preset ubsan-linux && cmake --build build-ubsan
ctest --test-dir build-ubsan
```

## Unit Tests

### Building and Running Tests

```bash
# Configure and build
cmake --preset debug-linux
cmake --build build

# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
ctest --test-dir build -R BlockingQueue --verbose

# Run tests in parallel
ctest --test-dir build -j $(nproc)
```

### Current Test Coverage

| Component | Tests | File |
|-----------|-------|------|
| File data source | 18 tests | `tests/data_sources/file_test.cpp` |
| Blocking queue | 8 tests | `tests/blocking_queue_test.cpp` |
| Media clock | 9 tests | `tests/renderers/media_clock_test.cpp` |
| **Total** | **35 tests** | |

### Writing New Tests

```cpp
#include <gtest/gtest.h>
#include "yapl/your_component.hpp"

TEST(YourComponentTest, DescriptiveName) {
    // Arrange: Set up test data
    your_component component;

    // Act: Perform operation
    auto result = component.do_something();

    // Assert: Verify expectations
    EXPECT_EQ(result, expected_value);
}
```

Add new test files to `tests/CMakeLists.txt`:
```cmake
target_sources(yapl_tests PRIVATE
    your_new_test.cpp
)
```

## Code Coverage

### Generating Coverage Reports

**Prerequisites**:
```bash
# Install coverage tools
sudo apt-get install lcov genhtml
```

**Workflow**:

1. **Build with coverage instrumentation**:
   ```bash
   cmake --preset coverage-linux
   cmake --build build-coverage
   ```

2. **Run tests to collect coverage data**:
   ```bash
   ctest --test-dir build-coverage --output-on-failure
   ```

3. **Generate HTML report**:
   ```bash
   ./scripts/generate_coverage.sh
   ```

4. **View report**:
   ```bash
   xdg-open build-coverage/coverage/index.html
   ```

### Manual Coverage Generation

```bash
# Capture coverage data
lcov --capture \
     --directory build-coverage \
     --output-file build-coverage/coverage.info \
     --exclude '/usr/*' \
     --exclude '*/tests/*' \
     --exclude '*/build-coverage/_deps/*'

# Generate HTML report
genhtml build-coverage/coverage.info \
        --output-directory build-coverage/coverage \
        --title "YAPL Code Coverage" \
        --legend \
        --show-details

# View summary
lcov --list build-coverage/coverage.info
```

### Coverage Targets

| Component | Target | Current |
|-----------|--------|---------|
| Core library | > 80% | TBD |
| Data sources | > 90% | ~85% (file_test.cpp) |
| Renderers | > 70% | ~30% (needs work) |
| Utilities | > 80% | TBD |

### Interpreting Coverage

- **Line coverage**: Percentage of code lines executed
- **Function coverage**: Percentage of functions called
- **Branch coverage**: Percentage of if/else branches taken

**Focus areas** (prioritize high-risk, low-coverage code):
- Error handling paths
- Concurrent operations
- Edge cases (empty queues, invalid inputs)

## Sanitizers

### AddressSanitizer (ASan)

**Detects**:
- Heap/stack/global buffer overflows
- Use-after-free
- Use-after-return
- Double-free
- Memory leaks

**Usage**:
```bash
# Build with ASan
cmake --preset asan-linux
cmake --build build-asan

# Run tests
ctest --test-dir build-asan

# Run specific executable
./build-asan/examples/yapl_player/yapl_player video.mp4

# Advanced options
ASAN_OPTIONS="detect_leaks=1:check_initialization_order=1:strict_init_order=1" \
    ./build-asan/examples/yapl_player/yapl_player video.mp4
```

**Common ASan Options**:
```bash
ASAN_OPTIONS="\
  detect_leaks=1:\ # Enable leak detection
  check_initialization_order=1:\ # Detect init order bugs
  strict_init_order=1:\ # Stricter checking
  halt_on_error=0:\ # Continue after first error
  log_path=asan.log\ # Write logs to file
"
```

**Reading ASan Output**:
```
ERROR: AddressSanitizer: heap-use-after-free on address 0x60400000eff0
READ of size 4 at 0x60400000eff0 thread T0
    #0 0x7f8a in function() src/file.cpp:42
    #1 0x7f9b in main() src/main.cpp:10
```
- Shows error type, address, stack trace
- Look for file:line in YOUR code (not system libraries)

### ThreadSanitizer (TSan)

**Detects**:
- Data races
- Deadlocks
- Thread leaks
- Signal-unsafe calls

**Usage**:
```bash
# Build with TSan
cmake --preset tsan-linux
cmake --build build-tsan

# Run tests
TSAN_OPTIONS="second_deadlock_stack=1" ctest --test-dir build-tsan

# Run with suppressions
TSAN_OPTIONS="suppressions=tsan.supp" ./build-tsan/tests/yapl_tests
```

**Common TSan Options**:
```bash
TSAN_OPTIONS="\
  second_deadlock_stack=1:\ # Show both deadlock stacks
  history_size=7:\ # Track more events (default: 2)
  halt_on_error=0:\ # Report all races
  report_bugs=1\ # Enable bug reporting
"
```

**Reading TSan Output**:
```
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 4 at 0x7b0400000020 by thread T2:
    #0 set_value() src/component.cpp:56
  Previous read of size 4 at 0x7b0400000020 by thread T1:
    #0 get_value() src/component.cpp:42
```
- Shows conflicting accesses and stack traces
- Fix by adding proper synchronization (mutex, atomic)

**Creating TSan Suppressions** (`tsan.supp`):
```
# Suppress known SDL internal races
race:SDL_*

# Suppress third-party library
race:^libavcodec*
```

**Known Limitations**:
- Cannot run with ASan simultaneously
- ~5-15x slowdown
- Requires position-independent code (-fPIE)

### UndefinedBehaviorSanitizer (UBSan)

**Detects**:
- Integer overflow/underflow
- Null pointer dereference
- Unaligned memory access
- Division by zero
- Shift errors
- Invalid enum values

**Usage**:
```bash
# Build with UBSan
cmake --preset ubsan-linux
cmake --build build-ubsan

# Run tests
ctest --test-dir build-ubsan

# With options
UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1" \
    ./build-ubsan/tests/yapl_tests
```

**Common UBSan Options**:
```bash
UBSAN_OPTIONS="\
  print_stacktrace=1:\ # Show stack traces
  halt_on_error=1:\ # Stop on first error
  suppressions=ubsan.supp\ # Suppression file
"
```

**Reading UBSan Output**:
```
runtime error: signed integer overflow: 2147483647 + 1 cannot be represented
    #0 0x4c8e5f in calculate() src/math.cpp:23
```
- Shows specific UB violation
- Fix by using proper types or checks

## Continuous Integration

### GitHub Actions Example

```yaml
name: Tests and Coverage

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        preset: [debug-linux, asan-linux, tsan-linux, ubsan-linux]
    steps:
      - uses: actions/checkout@v3
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y ninja-build libsdl2-dev libavcodec-dev
      - name: Build
        run: |
          cmake --preset ${{ matrix.preset }}
          cmake --build build-${{ matrix.preset }}
      - name: Test
        run: ctest --test-dir build-${{ matrix.preset }} --output-on-failure

  coverage:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y ninja-build libsdl2-dev lcov
      - name: Build with coverage
        run: |
          cmake --preset coverage-linux
          cmake --build build-coverage
      - name: Run tests
        run: ctest --test-dir build-coverage
      - name: Generate coverage
        run: ./scripts/generate_coverage.sh
      - name: Upload to Codecov
        uses: codecov/codecov-action@v3
        with:
          files: build-coverage/coverage.info
```

## Best Practices

### Test Development
1. **Write tests first** (TDD) for new features
2. **Test edge cases**: Empty inputs, null pointers, boundary values
3. **Use parameterized tests** for multiple similar cases
4. **Mock external dependencies** (files, network)
5. **Keep tests fast**: < 1 second per test

### Coverage Goals
1. **Aim for 80%+ coverage** on core library code
2. **Don't chase 100%**: Some code (error paths) is hard to test
3. **Focus on critical paths**: Decoders, renderers, synchronization
4. **Review uncovered lines**: Are they dead code or missing tests?

### Sanitizer Workflow
1. **Run ASan on every PR**: Catches most memory bugs
2. **Run TSan weekly**: Expensive but critical for threading
3. **Run UBSan in CI**: Fast and catches common mistakes
4. **Test with real workloads**: Not just unit tests
5. **Fix immediately**: Don't accumulate sanitizer warnings

### Performance Testing
1. **Benchmark before/after optimizations**: See `benchmarks/OPTIMIZATION_GUIDE.md`
2. **Profile with real data**: Not synthetic benchmarks only
3. **Use perf/gperftools**: Identify hotspots
4. **Test under load**: Concurrent playback, large files

## Troubleshooting

### Coverage: "No coverage data found"
```bash
# Ensure tests ran
ls -lh build-coverage/**/*.gcda

# Re-run tests
rm -rf build-coverage && cmake --preset coverage-linux
cmake --build build-coverage && ctest --test-dir build-coverage
```

### ASan: "Shadow memory range interleaves with existing memory mapping"
```bash
# Try with different memory layout
ASAN_OPTIONS="detect_leaks=0" ./build-asan/tests/yapl_tests
```

### TSan: "Failed to create new thread"
```bash
# Increase thread limit
ulimit -v unlimited
```

### UBSan: Too many warnings from third-party code
```bash
# Suppress external libraries
echo "unsigned-integer-overflow:libavcodec*" > ubsan.supp
UBSAN_OPTIONS="suppressions=ubsan.supp" ./build-ubsan/tests/yapl_tests
```

## References

- [Google Test Documentation](https://google.github.io/googletest/)
- [LCOV Coverage Tool](http://ltp.sourceforge.net/coverage/lcov.php)
- [AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- [ThreadSanitizer](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual)
- [UBSan Guide](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
