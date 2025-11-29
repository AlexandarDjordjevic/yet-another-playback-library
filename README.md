# YAPL - Yet Another Playback Library

A lightweight, modular C++20 library for building media player applications. YAPL provides a clean API for audio and video playback with extensible architecture for custom renderers and input handling.

## Features

- **Audio/Video Playback** - Synchronized A/V playback with PTS-based timing
- **Modular Architecture** - Pluggable renderers, decoders, and input handlers
- **Factory Pattern** - Easy to extend with custom implementations
- **Cross-platform** - Linux and Windows support
- **Pipeline Statistics** - Real-time buffer and progress monitoring

## Dependencies

| Library | Purpose |
|---------|---------|
| [FFmpeg](https://ffmpeg.org/) | Media demuxing and decoding |
| [SDL2](https://www.libsdl.org/) | Video/audio rendering and input |
| [curl](https://curl.se/) | HTTP streaming support |
| [spdlog](https://github.com/gabime/spdlog) | Logging |
| [fmt](https://github.com/fmtlib/fmt) | String formatting |

## Building

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt install libsdl2-dev libcurl4-openssl-dev libavformat-dev \
    libavcodec-dev libavutil-dev libswscale-dev libswresample-dev \
    libspdlog-dev libfmt-dev

# Fedora
sudo dnf install SDL2-devel libcurl-devel ffmpeg-devel spdlog-devel fmt-devel

# macOS
brew install sdl2 curl ffmpeg spdlog fmt
```

### Build with CMake Presets

```bash
# Clone
git clone https://github.com/AlexandarDjordjevic/yet-another-playback-library.git
cd yet-another-playback-library

# Configure and build (Linux)
cmake --preset debug-linux
cmake --build build

# Configure and build (Windows)
cmake --preset debug-windows
cmake --build build
```

### Manual Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Quick Start

### Basic Usage

```cpp
#include <yapl/player.hpp>
#include <yapl/renderers/sdl/video_renderer_factory.hpp>
#include <yapl/renderers/sdl/audio_renderer_factory.hpp>
#include <yapl/input/sdl/input_handler_factory.hpp>

int main() {
    // Create SDL-based factories
    auto video_factory = std::make_unique<yapl::renderers::sdl::video_renderer_factory>();
    auto audio_factory = std::make_unique<yapl::renderers::sdl::audio_renderer_factory>();
    auto input_factory = std::make_unique<yapl::input::sdl::input_handler_factory>();

    // Create player
    yapl::player player{
        std::move(video_factory),
        std::move(audio_factory),
        std::move(input_factory)
    };

    // Handle input commands
    player.set_command_callback([&](yapl::input::command cmd) {
        if (cmd == yapl::input::command::toggle_pause) {
            player.is_paused() ? player.resume() : player.pause();
        } else if (cmd == yapl::input::command::quit) {
            player.stop();
        }
    });

    // Load and play
    player.load("video.mp4");
    player.play();

    return 0;
}
```

### Link with CMake

```cmake
find_package(yapl REQUIRED)
target_link_libraries(my_app PRIVATE yapl::yapl)
```

## Examples

### yapl_player

A complete media player example with keyboard controls.

```bash
# Build
cmake --build build

# Run
./build/examples/yapl_player/yapl_player <media_file_or_url>

# Examples
./build/examples/yapl_player/yapl_player video.mp4
./build/examples/yapl_player/yapl_player http://example.com/stream.m3u8
```

**Controls:**
| Key | Action |
|-----|--------|
| `SPACE` | Toggle pause/resume |
| `S` | Show pipeline statistics |
| `Q` / `ESC` | Quit |

## Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `YAPL_LOG_LEVEL` | `info` | Log level: `trace`, `debug`, `info`, `warn`, `error`, `critical`, `off` |
| `YAPL_LOG_FILE` | - | Optional log file path |

```bash
# Enable debug logging
YAPL_LOG_LEVEL=debug ./yapl_player video.mp4

# Log to file
YAPL_LOG_FILE=/tmp/yapl.log ./yapl_player video.mp4
```

## Architecture

```
yapl/
├── player.hpp              # Main API - yapl::player
├── pipeline_stats.hpp      # Statistics and progress info
│
├── renderers/              # Video/Audio output
│   ├── i_video_renderer.hpp
│   ├── i_audio_renderer.hpp
│   ├── media_clock.hpp     # A/V sync clock
│   └── sdl/                # SDL2 implementation
│       ├── video_renderer_factory.hpp
│       └── audio_renderer_factory.hpp
│
├── input/                  # Input handling
│   ├── i_input_handler.hpp
│   └── sdl/                # SDL2 implementation
│       └── input_handler_factory.hpp
│
├── decoders/               # Media decoding
│   ├── i_decoder.hpp
│   └── ffmpeg/             # FFmpeg implementation
│
└── detail/                 # Internal implementation
    ├── media_pipeline.hpp
    ├── blocking_queue.hpp
    └── logger.hpp
```

### Namespaces

| Namespace | Purpose |
|-----------|---------|
| `yapl` | Core types: `player`, `pipeline_stats`, `media_info` |
| `yapl::renderers` | Renderer interfaces |
| `yapl::renderers::sdl` | SDL2 renderer implementations |
| `yapl::input` | Input handler interfaces and commands |
| `yapl::input::sdl` | SDL2 input implementation |
| `yapl::decoders` | Decoder interfaces |
| `yapl::decoders::ffmpeg` | FFmpeg decoder implementations |

## Pipeline Statistics

Monitor playback progress and buffer states:

```cpp
auto stats = player.get_stats();

// Progress
stats.progress.position_ms;      // Current position
stats.progress.duration_ms;      // Total duration
stats.progress.progress_percent(); // 0-100%

// Buffer queues
stats.video_track_queue.size;    // Samples waiting to decode
stats.video_renderer_queue.size; // Frames waiting to render
stats.audio_track_queue.size;
stats.audio_renderer_queue.size;

// Formatted output
LOG_INFO("{}", stats.to_string());
// "1:23 / 5:45 (25%) | Source: 512KB | VTrack: 45/1024 (4%) | ..."
```

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `YAPL_BUILD_TESTS` | `ON` | Build unit tests |
| `YAPL_BUILD_EXAMPLES` | `ON` | Build examples |

## License

MIT License. See [LICENSE](LICENSE) for details.

## Contributing

Contributions welcome! Please open issues or submit pull requests.
