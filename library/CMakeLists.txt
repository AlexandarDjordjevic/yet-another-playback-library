set(SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/data_sources/file_data_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/media_source.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/ffmpeg_media_extractor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/decoders/ffmpeg/audio_decoder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/decoders/ffmpeg/video_decoder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/media_pipeline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/player.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/yapl/track.cpp
)

set(INCLUDE_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/renderers/i_video_renderer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/renderers/i_video_renderer_factory.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/renderers/i_audio_renderer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/renderers/i_audio_renderer_factory.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/data_sources/file_data_source.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/debug.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/blocking_queue.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/ffmpeg_media_extractor.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/yapl/decoders/ffmpeg/audio_decoder.hpp
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
find_package(SDL2 REQUIRED)

# Platform-specific FFmpeg handling
if(WIN32)
    message(STATUS "Windows: Using vcpkg for FFmpeg")
    
    # Try to find FFmpeg components
    find_path(FFMPEG_INCLUDE_DIR
        NAMES libavcodec/avcodec.h
        PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/include
        NO_DEFAULT_PATH
    )
    
    find_library(AVCODEC_LIB avcodec
        PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/lib
        NO_DEFAULT_PATH
    )
    
    find_library(AVFORMAT_LIB avformat
        PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/lib
        NO_DEFAULT_PATH
    )
    
    find_library(AVUTIL_LIB avutil
        PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/lib
        NO_DEFAULT_PATH
    )
    
    find_library(SWSCALE_LIB swscale
        PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/lib
        NO_DEFAULT_PATH
    )
    
    find_library(SWRESAMPLE_LIB swresample
        PATHS ${VCPKG_INSTALLED_DIR}/x64-windows/lib
        NO_DEFAULT_PATH
    )
    
    set(FFMPEG_LIBRARIES 
        ${AVCODEC_LIB}
        ${AVFORMAT_LIB} 
        ${AVUTIL_LIB}
        ${SWSCALE_LIB}
        ${SWRESAMPLE_LIB}
    )
    
else()
    # Linux: Use pkg-config for FFmpeg
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED 
        libavformat 
        libavcodec 
        libavutil 
        libswscale 
        libswresample
    )
endif()

# Create the static library - include both source and header files
add_library(yapl STATIC ${SOURCE_FILES} ${HEADER_FILES})

# Set target properties to make it visible
set_target_properties(yapl PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${HEADER_FILES}"
)

# Include headers
target_include_directories(yapl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Common linking for all platforms
target_link_libraries(yapl PUBLIC
    spdlog::spdlog
    fmt::fmt
)

# Platform-specific FFmpeg linking
if(WIN32)
    target_include_directories(yapl PUBLIC ${FFMPEG_INCLUDE_DIR})
    target_link_libraries(yapl PUBLIC ${FFMPEG_LIBRARIES})
else()
    target_link_libraries(yapl PUBLIC ${FFMPEG_LIBRARIES})
    target_include_directories(yapl PUBLIC ${FFMPEG_INCLUDE_DIRS})
    target_compile_options(yapl PUBLIC ${FFMPEG_CFLAGS_OTHER})
endif()

# Set C++ standard
set_target_properties(yapl PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Handle Windows-specific settings
if(WIN32)
    target_compile_definitions(yapl PRIVATE _CRT_SECURE_NO_WARNINGS)
    if(MSVC)
        target_compile_options(yapl PRIVATE /W4)
    endif()
endif()

export(TARGETS yapl FILE "${CMAKE_CURRENT_BINARY_DIR}/yaplTargets.cmake")
