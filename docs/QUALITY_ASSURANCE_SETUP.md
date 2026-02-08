# Quality Assurance Setup Summary

This document summarizes the testing and quality assurance infrastructure added to YAPL.

## Overview

A comprehensive QA infrastructure has been implemented to ensure code quality, detect bugs early, and track test coverage:

1. **Code Coverage** - Measure and track test coverage
2. **Sanitizers** - Detect memory errors, race conditions, and undefined behavior
3. **CI/CD** - Automated testing on every commit
4. **Documentation** - Complete testing guides and best practices

## Components Added

### CMake Presets

Four new CMake presets for quality assurance:

| Preset | Purpose | Flags |
|--------|---------|-------|
| `coverage-linux` | Code coverage | `--coverage -fprofile-arcs -ftest-coverage` |
| `asan-linux` | Memory error detection | `-fsanitize=address -fno-omit-frame-pointer` |
| `tsan-linux` | Race condition detection | `-fsanitize=thread` |
| `ubsan-linux` | Undefined behavior detection | `-fsanitize=undefined` |

**Usage:**
```bash
# Coverage
cmake --preset coverage-linux && cmake --build build-coverage
ctest --test-dir build-coverage
./scripts/generate_coverage.sh

# Sanitizers
cmake --preset asan-linux && cmake --build build-asan
ctest --test-dir build-asan
```

### Helper Scripts

**scripts/generate_coverage.sh**
- Automated coverage report generation
- Uses lcov/genhtml for HTML reports
- Filters out system headers and third-party code
- Color-coded summary output
- Calculates overall coverage percentage

**scripts/run_sanitizers.sh**
- Run all sanitizers in sequence
- Configurable per-sanitizer execution
- Summary report of all results
- Exit code for CI integration

**Usage:**
```bash
# Generate coverage report
./scripts/generate_coverage.sh

# Run all sanitizers
./scripts/run_sanitizers.sh

# Run specific sanitizer
./scripts/run_sanitizers.sh asan
```

### GitHub Actions CI

**Workflow: .github/workflows/ci.yml**

Runs on every push and pull request to master/develop:

1. **Build and Test** - Standard debug build
2. **AddressSanitizer** - Memory error detection
3. **ThreadSanitizer** - Race condition detection
4. **UndefinedBehaviorSanitizer** - UB detection
5. **Code Coverage** - Generate and upload to Codecov
6. **Benchmarks** - Performance regression detection

**Features:**
- Parallel execution of all jobs
- Automatic artifact upload
- Codecov integration
- Status badges in README

### Documentation

**docs/TESTING_GUIDE.md** (comprehensive guide)
- Quick reference for all testing tools
- Detailed sanitizer usage and options
- Coverage report generation
- Troubleshooting common issues
- CI integration examples
- Best practices

**README.md updates**
- Status badges (CI, Codecov, License)
- Testing and Quality Assurance section
- Quick start examples
- Links to detailed guides

### .gitignore Updates

Added patterns to exclude build artifacts:
```
build-*/           # All build variants
*.gcda, *.gcno     # Coverage data
*.gcov, *.info     # Coverage reports
coverage/          # Coverage HTML output
benchmark_*.json   # Benchmark results
```

## Current Test Coverage

| Component | Test Count | File |
|-----------|-----------|------|
| File data source | 18 tests | `tests/data_sources/file_test.cpp` |
| Blocking queue | 8 tests | `tests/blocking_queue_test.cpp` |
| Media clock | 9 tests | `tests/renderers/media_clock_test.cpp` |
| **Total** | **35 tests** | All passing ✅ |

## Workflow Integration

### Development Workflow

1. **Make changes** to code
2. **Run local tests** with `ctest`
3. **Check coverage** with `./scripts/generate_coverage.sh`
4. **Run sanitizers** with `./scripts/run_sanitizers.sh`
5. **Commit** and push
6. **CI runs** all checks automatically

### Pull Request Workflow

1. Create PR from feature branch
2. GitHub Actions runs all checks
3. Codecov comments with coverage diff
4. Review code and test results
5. Merge when all checks pass

### Release Workflow

1. Run full test suite including sanitizers
2. Generate and review coverage report
3. Run performance benchmarks
4. Compare against baseline metrics
5. Tag release

## Next Steps

### Immediate Improvements

1. **Increase test coverage**
   - Target: 80%+ line coverage
   - Add renderer tests with mocked dependencies
   - Add integration tests for full pipeline

2. **Add benchmark regression detection**
   - Store baseline benchmarks
   - Compare on each PR
   - Alert on >10% performance degradation

3. **Enhance CI**
   - Add Windows builds
   - Add macOS builds
   - Matrix testing with different compiler versions

### Long-term Improvements

1. **Static analysis**
   - clang-tidy integration
   - cppcheck integration
   - Add to CI pipeline

2. **Fuzzing**
   - libFuzzer for decoders
   - OSS-Fuzz integration
   - Corpus management

3. **Performance monitoring**
   - Continuous benchmark tracking
   - Performance dashboard
   - Historical trend analysis

## Maintenance

### Weekly Tasks

- [ ] Review coverage reports
- [ ] Check for new sanitizer warnings
- [ ] Update baseline benchmarks if improved

### Monthly Tasks

- [ ] Review and update test cases
- [ ] Analyze coverage gaps
- [ ] Update CI configuration
- [ ] Review and fix TODOs in code

### Release Tasks

- [ ] Run full sanitizer suite
- [ ] Generate final coverage report
- [ ] Run performance benchmarks
- [ ] Update CHANGELOG with QA improvements

## Resources

- [TESTING_GUIDE.md](TESTING_GUIDE.md) - Complete testing documentation
- [benchmarks/OPTIMIZATION_GUIDE.md](../benchmarks/OPTIMIZATION_GUIDE.md) - Performance optimization guide
- [Google Test](https://google.github.io/googletest/) - Unit testing framework
- [Google Benchmark](https://github.com/google/benchmark) - Benchmarking framework
- [AddressSanitizer](https://github.com/google/sanitizers/wiki/AddressSanitizer) - Memory error detector
- [ThreadSanitizer](https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual) - Race detector
- [Codecov](https://about.codecov.io/) - Code coverage service

## Questions?

See the [TESTING_GUIDE.md](TESTING_GUIDE.md) for detailed instructions, or open an issue on GitHub.
