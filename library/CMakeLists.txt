set(SOURCE_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/data_sources/file_data_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/media_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/ffmpeg_media_extractor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/decoders/ffmpeg/video_decoder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/media_pipeline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/player.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/track.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/renderers/sdl/video_renderer.cpp
)

set(INCLUDE_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/renderers/sdl/video_renderer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/data_sources/file_data_source.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/debug.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/ffmpeg_media_extractor.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/decoders/ffmpeg/video_decoder.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/idata_source.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/decoders/idecoder.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/imedia_extractor.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/imedia_source.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/logger.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/media_info.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/media_pipeline.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/media_sample.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/media_source.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/player.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/track.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/track_info.hpp
)

# Find external dependencies
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)

# Find individual FFmpeg libraries (create one imported target per module)
# Find individual FFmpeg libraries via pkg-config and collect flags
# Use pkg_check_modules to populate <NAME>_LIBRARIES and <NAME>_INCLUDE_DIRS
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SWSCALE REQUIRED libswscale)
pkg_check_modules(SWRESAMPLE REQUIRED libswresample)

message(STATUS "AVCODEC_LIBRARIES=${AVCODEC_LIBRARIES}")
message(STATUS "AVFORMAT_LIBRARIES=${AVFORMAT_LIBRARIES}")
message(STATUS "AVUTIL_LIBRARIES=${AVUTIL_LIBRARIES}")
message(STATUS "SWSCALE_LIBRARIES=${SWSCALE_LIBRARIES}")
message(STATUS "SWRESAMPLE_LIBRARIES=${SWRESAMPLE_LIBRARIES}")

# Create the shared library
add_library(yapl SHARED ${SOURCE_FILES} ${INCLUDE_FILES})

message(INFO "CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR}")
# Include headers
target_include_directories(yapl PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

find_package(SDL2 REQUIRED)

target_link_libraries(yapl
    PUBLIC
    spdlog::spdlog
    fmt::fmt
    ${AVCODEC_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${SWSCALE_LIBRARIES}
    ${SWRESAMPLE_LIBRARIES}
    ${SDL2_LIBRARIES}
)

## Add FFmpeg include directories and any extra cflags from pkg-config
target_include_directories(yapl PUBLIC
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVCODEC_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${SWSCALE_INCLUDE_DIRS}
    ${SWRESAMPLE_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)
target_compile_options(yapl PUBLIC ${AVFORMAT_CFLAGS_OTHER} ${AVCODEC_CFLAGS_OTHER} ${AVUTIL_CFLAGS_OTHER} ${SWSCALE_CFLAGS_OTHER} ${SWRESAMPLE_CFLAGS_OTHER})
