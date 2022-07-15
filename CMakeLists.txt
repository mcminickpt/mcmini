cmake_minimum_required(VERSION 3.10)

project(McMini 
    VERSION 0.0.0
    DESCRIPTION "A bite-sized model checker"
    LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GoogleTest)
include(McMiniTest)
include(GitSubmodule)

set(CMAKE_GENERATOR "Unix Makefiles")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(MCMINI_DIR ${CMAKE_SOURCE_DIR})
set(MCMINI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(MCMINI_GOOGLE_TEST_DIR ${CMAKE_SOURCE_DIR}/external/googletest)
set(MCMINI_GOOGLE_TEST_INCLUDE_DIR ${MCMINI_GOOGLE_TEST_DIR}/include)

include_directories(BEFORE ${MCMINI_INCLUDE_DIR})

# MARK: - CTest Setup -
option(VERBOSE_TESTING OFF)
option(BUILD_TESTS OFF)
enable_testing()

# ... Other testing steps here (custom test scripts etc.)

# MARK - CPack Setup -

# set CPACK_* variables before include()-ing the CPack module

add_subdirectory(src)
add_subdirectory(external)

if (BUILD_TESTS)
add_subdirectory(tests)
endif()