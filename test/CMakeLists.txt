project(DPOR_Tests)

cmake_minimum_required(VERSION 3.21)

# 'lib' is the folder with Google Test sources (header files, etc)
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Add test C++ files here
file(GLOB DPOR_PROJECT_SOURCE_FILES CONFIGURE_DEPENDS "../src/*.c" "../src/*.h")
message(${DPOR_PROJECT_SOURCE_FILES})

#set(DPOR_PROJECT_SOURCE_FILES ../src/array.c ../src/hashtable.c ../src/transition.c)
add_executable(DPOR_Tests_Run array_tests.cpp test_common.h hashtable_tests.cpp ${DPOR_PROJECT_SOURCE_FILES} transition_enabled_tests.cpp)

set(DPOR_TEST_LINK_TARGET_FLAGS -lrt -pthread -lm)
target_link_libraries(DPOR_Tests_Run gtest gtest_main)
target_link_libraries(DPOR_Tests_Run ${DPOR_TEST_LINK_TARGET_FLAGS})


