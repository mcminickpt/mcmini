cmake_minimum_required(VERSION 3.10)
project(McMini
    VERSION 0.0.1
    DESCRIPTION "A bite-sized model checker"
    LANGUAGES C CXX)

# C and C++ Versions
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Project configuration
option(BUILD_TESTS OFF)
option(VERBOSE_TESTING OFF)
set(MCMINI_DIR "${CMAKE_SOURCE_DIR}")
set(MCMINI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(MCMINI_CMAKE_MODULE_DIR "${CMAKE_SOURCE_DIR}/cmake")
set(MCMINI_SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/script")
set(MCMINI_DOCS_DIR "${CMAKE_SOURCE_DIR}/docs")
set(MCMINI_GOOGLE_TEST_DIR "${CMAKE_SOURCE_DIR}/external/googletest")
set(MCMINI_GOOGLE_TEST_INCLUDE_DIR "${MCMINI_GOOGLE_TEST_DIR}/include")

# Include the "cmake/" directory as a location to look for CMake modules
list(APPEND CMAKE_MODULE_PATH "${MCMINI_CMAKE_MODULE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${MCMINI_CMAKE_MODULE_DIR}/cpack")

# McMini cmake modules
include(PreCommit)
include(McMiniTest)
include(GitSubmodule)
include(CMakeConditionals)
include(ConfigureScripts)

add_subdirectory(src)
add_subdirectory(external)
add_subdirectory_if(BUILD_TESTS "Tests only built with BUILD_TESTS=1" tests)

# CTest setup
enable_testing()

# CPack Setup

set(CPACK_VERBATIM_VARIABLES TRUE)
set(CPACK_INSTALLATION_DIR "installation")
set(CPACK_PACKAGE_NAME "McMini")
set(CPACK_PACKAGE_VENDOR "Khoury College")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/${CPACK_INSTALLATION_DIR})
set(CPACK_DESCRIPTION_FILE ${MCMINI_DOCS_DIR}/PackageDescription.txt)
set(CPACK_RESOURCE_FILE_WELCOME ${MCMINI_DOCS_DIR}/Welcome.txt)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A bite-sized model checker")
set(CPACK_PACKAGE_CHECKSUM SHA512)
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README.md)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
include(CPackIgnore)
include(CPack)



