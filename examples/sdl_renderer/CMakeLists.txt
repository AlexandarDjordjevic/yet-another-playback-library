
find_package(SDL2 CONFIG REQUIRED)

set(YAPL_PLAYER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/source/sdl/audio_renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/sdl/video_renderer.cpp
)

set(YAPL_PLAYER_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sdl/audio_renderer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sdl/audio_renderer_factory.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sdl/video_renderer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sdl/video_renderer_factory.hpp
)

add_executable(
    yapl_player 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${YAPL_PLAYER_SRC}
    ${YAPL_PLAYER_INC}
)

add_dependencies(yapl_player yapl)

target_include_directories(yapl_player PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(yapl_player
    PRIVATE
    yapl
    SDL2::SDL2main
    SDL2::SDL2
)
