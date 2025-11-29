cmake_minimum_required(VERSION 3.21)

project(yapl
    VERSION 0.1.0
    DESCRIPTION "Yet Another Playback Library"
    LANGUAGES CXX
)

# Global C++ settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(YAPL_BUILD_TESTS "Build unit tests" ON)
option(YAPL_BUILD_EXAMPLES "Build examples" ON)

add_subdirectory(library)

if(YAPL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(YAPL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
