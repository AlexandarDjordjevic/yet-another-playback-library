find_package(GTest REQUIRED)

add_executable(yapl_tests
    data_sources/file_test.cpp
    blocking_queue_test.cpp
    renderers/media_clock_test.cpp
)

target_link_libraries(yapl_tests
    PRIVATE
        yapl::yapl
        GTest::gtest
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(yapl_tests)
