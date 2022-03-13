project(DPOR_Tests)

# 'lib' is the folder with Google Test sources (header files, etc)
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Add test C++ files here

set(DPOR_PROJECT_SOURCE_FILES ../src/array.c ../src/hashtable.c)
add_executable(DPOR_Tests_Run array_tests.cpp test_common.h hashtable_tests.cpp ${DPOR_PROJECT_SOURCE_FILES})
target_link_libraries(DPOR_Tests_Run gtest gtest_main)

