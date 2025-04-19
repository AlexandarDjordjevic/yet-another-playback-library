set(SOURCE_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/source/sklepan/FileDataSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/sklepan/MediaSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/sklepan/FFMpegMediaExtractor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/sklepan/MediaPipeline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/source/sklepan/Player.cpp
)

# Find external dependencies
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)

# Find individual FFmpeg libraries
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavformat libavcodec libavutil libswscale libswresample)

# Create the shared library
add_library(sklepan SHARED ${SOURCE_FILES})

# Include headers
target_include_directories(sklepan PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link with all required libraries
target_link_libraries(sklepan
    PUBLIC
        spdlog::spdlog
        PkgConfig::FFMPEG
)
