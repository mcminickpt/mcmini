project(DPOR_Tests)

# 'lib' is the folder with Google Test sources (header files, etc)
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Add test C++ files here
add_executable(DPOR_Tests_Run array_tests.cpp ../src/array.c)
target_link_libraries(DPOR_Tests_Run gtest gtest_main)

