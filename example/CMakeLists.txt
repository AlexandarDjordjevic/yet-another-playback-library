
add_executable(example_main main.cpp)

target_link_libraries(example_main 
    PRIVATE 
    sklepan
    sfml-graphics sfml-window sfml-system
)
