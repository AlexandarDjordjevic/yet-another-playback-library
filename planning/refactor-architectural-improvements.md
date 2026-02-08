# Refactor: YAPL Architectural Improvements

## Goal
Address critical architectural issues in YAPL to improve testability, robustness, and maintainability:
1. **PR 1**: Remove global singleton `media_clock` and inject as dependency
2. **PR 2**: Add RAII wrappers for SDL resources to prevent leaks
3. **PR 3**: Make queue sizes configurable
4. **PR 4**: Expand test coverage (blocking_queue, renderers, integration tests)

**Out of scope** (deferred to future work):
- PR 5: Error handling improvements
- PR 6: Seek support implementation

## Analysis Summary

### Code Smells Identified
- [x] **Singleton Anti-Pattern**: `media_clock::instance()` used in video_renderer.cpp:79, 84, 91, 96 and audio_renderer.cpp:71
  - Breaks multi-instance player usage
  - Not testable (can't mock for tests)
  - Thread-unsafe for concurrent players

- [x] **Resource Leak Risk**: SDL resources in video_renderer.cpp:16-33 and audio_renderer.cpp:31-52
  - Raw pointers (SDL_Window*, SDL_Renderer*, SDL_Texture*, SDL_AudioDeviceID)
  - Exception in resize() (line 46-67) could leak partially allocated resources
  - No RAII wrappers

- [x] **Inconsistent Error Handling**: 8 files throw std::runtime_error
  - Errors not propagated to user (logged only)
  - Decoder failures continue playback silently
  - No error callback mechanism

- [x] **Limited Test Coverage**: Only tests/data_sources/file_test.cpp (18 tests)
  - No tests for blocking_queue (critical thread-safe component)
  - No tests for renderers
  - No integration tests

- [x] **Hard-Coded Constraints**: Queue sizes fixed in code
  - `blocking_queue<std::shared_ptr<media_sample>> m_frames{60}` in renderers
  - Track queues: 1024 items (hard-coded in media_pipeline)
  - No performance tuning capability

## Refactoring Strategy

### Best Practices to Apply
- **I.1**: Make interfaces explicit - inject media_clock dependency
- **C.31**: Prefer RAII over manual resource management
- **F.6**: Prefer return values to out-parameters and exceptions
- **E.27**: Use smart pointers to represent ownership
- **T.1**: Use templates to express type-independent code
- **SL.con.1**: Prefer using STL array or vector over raw arrays

### Design Patterns to Use
- **Dependency Injection**: Pass media_clock reference to renderers instead of singleton
- **RAII**: Wrap SDL resources in smart classes (sdl_window_handle, sdl_renderer_handle, etc.)
- **Builder/Config Pattern**: Create configuration struct for tunable parameters
- **Strategy Pattern** (existing): Maintain pluggable renderer architecture

### STL Algorithms to Apply
No algorithm replacements needed (existing code uses appropriate constructs)

## PR Strategy

This refactoring will be split into **multiple PRs** for easier review:

### PR 1: Remove media_clock Singleton (~200 lines)
**Scope:** Dependency injection for media_clock
**Dependencies:** None
**Files:**
- `library/include/yapl/renderers/media_clock.hpp` - remove singleton, make constructible
- `library/include/yapl/renderers/i_video_renderer.hpp` - add clock parameter
- `library/include/yapl/renderers/i_audio_renderer.hpp` - add clock parameter
- `library/include/yapl/renderers/sdl/video_renderer.hpp` - store clock reference
- `library/include/yapl/renderers/sdl/audio_renderer.hpp` - store clock reference
- `library/source/yapl/renderers/sdl/video_renderer.cpp` - use injected clock
- `library/source/yapl/renderers/sdl/audio_renderer.cpp` - use injected clock
- `library/include/yapl/renderers/sdl/video_renderer_factory.hpp` - pass clock to constructor
- `library/include/yapl/renderers/sdl/audio_renderer_factory.hpp` - pass clock to constructor
- `library/source/yapl/media_pipeline.cpp` - create clock, pass to factories

### PR 2: SDL Resource RAII Wrappers (~300 lines)
**Scope:** Safe SDL resource management
**Dependencies:** None (independent from PR 1)
**Files:**
- `library/include/yapl/detail/sdl_resource_handles.hpp` - NEW: RAII wrappers
- `library/source/yapl/renderers/sdl/video_renderer.cpp` - use RAII handles
- `library/source/yapl/renderers/sdl/audio_renderer.cpp` - use RAII handles
- `library/include/yapl/renderers/sdl/video_renderer.hpp` - replace raw pointers

### PR 3: Configurable Queue Sizes (~150 lines)
**Scope:** Configuration struct for tuning
**Dependencies:** None
**Files:**
- `library/include/yapl/pipeline_config.hpp` - NEW: configuration options
- `library/include/yapl/player.hpp` - add config parameter
- `library/include/yapl/renderers/i_video_renderer.hpp` - queue size param
- `library/include/yapl/renderers/i_audio_renderer.hpp` - queue size param
- `library/source/yapl/media_pipeline.cpp` - use config values

### PR 4: Expanded Test Coverage (~400 lines)
**Scope:** Unit tests for core components
**Dependencies:** PR 1 (for testable clock)
**Files:**
- `tests/blocking_queue_test.cpp` - NEW: thread-safety tests
- `tests/renderers/video_renderer_test.cpp` - NEW: mock clock tests
- `tests/renderers/audio_renderer_test.cpp` - NEW: mock clock tests
- `tests/integration/pipeline_test.cpp` - NEW: end-to-end test

### PR 5: Error Handling (DEFERRED - Future work)
**Scope:** Error propagation framework
**Status:** Not included in current implementation scope

### PR 6: Seek Support (DEFERRED - Future work)
**Scope:** Media seeking implementation
**Status:** Not included in current implementation scope

## Detailed Tasks for PR 1 (Remove Singleton Clock)

- [ ] Phase 1: Modify media_clock interface
  - [ ] Remove `static instance()` method from media_clock.hpp
  - [ ] Make constructor public
  - [ ] Add move constructor and assignment (delete copy)

- [ ] Phase 2: Update renderer interfaces
  - [ ] Add `media_clock&` parameter to renderer constructors (cleaner approach)
  - [ ] Update i_video_renderer interface
  - [ ] Update i_audio_renderer interface

- [ ] Phase 3: Update SDL implementations
  - [ ] Add `media_clock& m_clock` member to video_renderer
  - [ ] Update constructor to accept `media_clock&` parameter
  - [ ] Replace all `media_clock::instance()` calls with `m_clock`
  - [ ] Repeat for audio_renderer

- [ ] Phase 4: Update factories
  - [ ] Add `media_clock&` parameter to create_video_renderer()
  - [ ] Update video_renderer_factory::create_video_renderer() implementation
  - [ ] Repeat for audio_renderer_factory

- [ ] Phase 5: Update media_pipeline
  - [ ] Create `media_clock` instance in media_pipeline constructor or member
  - [ ] Pass clock reference when creating renderers via factories
  - [ ] Verify clock lifecycle (owned by pipeline, outlives renderers)

- [ ] Phase 6: Verify and test
  - [ ] Build succeeds with no warnings
  - [ ] Run existing example: `examples/yapl_player`
  - [ ] Verify A/V sync still works correctly
  - [ ] Run all existing tests

## Testability Improvements

After refactoring:
- **media_clock** can be mocked for renderer unit tests
- **SDL resources** won't leak even if tests throw exceptions
- **Renderers** can be tested independently with fake clocks
- **Pipeline** can be tested with configurable queue sizes
- **Multiple players** can run concurrently without singleton conflicts

## Changelog

### Changed
- **BREAKING**: `media_clock` is no longer a singleton - inject via renderer factories
- Renderer constructors now require `media_clock&` parameter
- Added RAII wrappers for SDL resources (prevents leaks)
- Queue sizes now configurable via `pipeline_config`
- Expanded test coverage for thread-safe components

## Notes

### Decision: Clock Injection Strategy
**Choice**: Pass clock to renderer constructor (cleaner interface, clock stored as member)
```cpp
video_renderer(media_clock& clock) : m_clock(clock) {}
```

### Decision: RAII Wrapper Design
Create minimal wrapper classes:
```cpp
class sdl_window_handle {
    SDL_Window* m_window;
public:
    explicit sdl_window_handle(const char* title, int w, int h);
    ~sdl_window_handle() { if (m_window) SDL_DestroyWindow(m_window); }
    SDL_Window* get() const { return m_window; }
    // Move-only
};
```

### Decision: Queue Size Configuration
Use struct for extensibility:
```cpp
struct pipeline_config {
    size_t video_queue_size = 60;
    size_t audio_queue_size = 60;
    size_t track_queue_size = 1024;
    size_t http_buffer_min_kb = 512;
};
```

## Test Plan

After EACH PR:
1. Build: `cmake --preset debug-linux && cmake --build build`
2. Run tests: `ctest --test-dir build`
3. Run example: `./build/examples/yapl_player/yapl_player <video_file>`
4. Verify:
   - Video plays smoothly
   - Audio synchronized
   - No crashes on resize
   - No memory leaks (run with valgrind)
   - Pause/resume works correctly

## Verification Steps

### PR 1 Verification (Singleton Removal):
```bash
# 1. Build
cmake --preset debug-linux
cmake --build build

# 2. Run tests
ctest --test-dir build --output-on-failure

# 3. Test with example
./build/examples/yapl_player/yapl_player test_video.mp4

# 4. Check for memory issues
valgrind --leak-check=full ./build/examples/yapl_player/yapl_player test.mp4
```

### PR 2 Verification (RAII):
```bash
# Run with exceptions enabled in destructors to catch leaks
# No SDL resources should leak during cleanup
```

### PR 3 Verification (Config):
```bash
# Test with different queue sizes in player constructor
```

### PR 4 Verification (Tests):
```bash
# All new tests should pass
ctest --test-dir build --output-on-failure

# Check coverage (optional)
cmake --preset debug-linux -DCMAKE_CXX_FLAGS="--coverage"
cmake --build build
ctest --test-dir build
gcovr -r . --html-details coverage.html
```
