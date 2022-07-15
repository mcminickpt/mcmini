cmake_minimum_required(VERSION 3.10)

project(McMini 
    VERSION 0.0.0
    DESCRIPTION "A bite-sized model checker"
    LANGUAGES C CXX)

# MARK: - CMake Setup -

# Add to the CMake module include path any custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include directories come BEFORE other directories in the include path
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

set(CMAKE_GENERATOR "Unix Makefiles")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

#... Set other cmake variables here, or perahps include



set(MCMINI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(MCMINI_GOOGLE_TEST_DIR ${CMAKE_SOURCE_DIR}/external/googletest)
set(MCMINI_GOOGLE_TEST_INCLUDE_DIR ${MCMINI_GOOGLE_TEST_DIR}/include)

include_directories(${MCMINI_INCLUDE_DIR})

# MARK: - CTest Setup -
option(BUILD_TESTS OFF)
enable_testing()

# ... Other testing steps here (custom test scripts etc.)

# MARK - CPack Setup -

# set CPACK_* variables before include()-ing the CPack module

message("${MCMINI_GOOGLE_TEST_DIR} asdf")

include(GoogleTest)
include(McMiniTest)

add_subdirectory(src)
add_subdirectory(external)

if (BUILD_TESTS)
add_subdirectory(tests)
endif()

message("${MCMINI_GOOGLE_TEST_DIR} asdf")


