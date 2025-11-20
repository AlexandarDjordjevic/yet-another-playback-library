
project(yapl_player)

add_executable(yapl_player ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_dependencies(yapl_player yapl)

target_link_libraries(yapl_player 
    PRIVATE 
    yapl
)
