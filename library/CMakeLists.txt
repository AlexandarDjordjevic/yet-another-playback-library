# Source files
set(YAPL_SOURCES
    source/yapl/data_sources/file.cpp
    source/yapl/data_sources/http.cpp
    source/yapl/decoders/ffmpeg/audio_decoder.cpp
    source/yapl/decoders/ffmpeg/video_decoder.cpp
    source/yapl/ffmpeg_media_extractor.cpp
    source/yapl/media_pipeline.cpp
    source/yapl/media_source.cpp
    source/yapl/player.cpp
    source/yapl/track.cpp
    source/yapl/renderers/sdl/video_renderer.cpp
    source/yapl/renderers/sdl/audio_renderer.cpp
    source/yapl/input/sdl/input_handler.cpp
)

# Find dependencies
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)

if(WIN32)
    message(STATUS "Windows: Using vcpkg for FFmpeg and CURL")
    find_package(CURL REQUIRED)

    find_path(FFMPEG_INCLUDE_DIR
        NAMES libavcodec/avcodec.h
        PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/include
        NO_DEFAULT_PATH
    )

    find_library(AVCODEC_LIB avcodec PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/lib NO_DEFAULT_PATH)
    find_library(AVFORMAT_LIB avformat PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/lib NO_DEFAULT_PATH)
    find_library(AVUTIL_LIB avutil PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/lib NO_DEFAULT_PATH)
    find_library(SWSCALE_LIB swscale PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/lib NO_DEFAULT_PATH)
    find_library(SWRESAMPLE_LIB swresample PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/lib NO_DEFAULT_PATH)

    set(FFMPEG_LIBRARIES ${AVCODEC_LIB} ${AVFORMAT_LIB} ${AVUTIL_LIB} ${SWSCALE_LIB} ${SWRESAMPLE_LIB})
    set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIR})
    set(CURL_LIBRARIES CURL::libcurl)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
        libavformat
        libavcodec
        libavutil
        libswscale
        libswresample
    )
    pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)
endif()

# Create library target
add_library(yapl STATIC ${YAPL_SOURCES})
add_library(yapl::yapl ALIAS yapl)

# Include directories
target_include_directories(yapl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${FFMPEG_INCLUDE_DIRS}
)

# Link libraries
if(WIN32)
    target_link_libraries(yapl
        PUBLIC
            spdlog::spdlog
            fmt::fmt
            SDL2::SDL2
            SDL2::SDL2main
        PRIVATE
            ${FFMPEG_LIBRARIES}
            ${CURL_LIBRARIES}
    )
else()
    target_link_libraries(yapl
        PUBLIC
            spdlog::spdlog
            fmt::fmt
            SDL2::SDL2
            SDL2::SDL2main
        PRIVATE
            PkgConfig::FFMPEG
            PkgConfig::CURL
    )
endif()

# Compiler options
target_compile_options(yapl
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

target_compile_definitions(yapl
    PRIVATE
        $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
)

# Target properties
set_target_properties(yapl PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Export for FetchContent/add_subdirectory users
export(TARGETS yapl
    NAMESPACE yapl::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/yaplTargets.cmake"
)
