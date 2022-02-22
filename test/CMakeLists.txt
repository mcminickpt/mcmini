project(DPOR_Tests)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(DPOR_Tests_Run)
target_link_libraries(DPOR_Tests_Run gtest gtest_main)