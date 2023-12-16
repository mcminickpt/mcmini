cmake_minimum_required(VERSION 3.10)
project(McMini-Revamp
    VERSION 1.0.0
    DESCRIPTION "A bite-sized model checker"
    LANGUAGES C CXX)

# Require C98 and C++11
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

# Project source files
set(MCMINI_C_SRC )
set(MCMINI_CPP_SRC
  src/mcmini/mcmini.cpp
  src/mcmini/visible_object.cpp
  src/mcmini/model/state_sequence.cpp
  src/mcmini/model/detached_state.cpp
)
set(LIBMCMINI_C_SRC )
set(LIBMCMINI_CPP_SRC
  src/lib/entry.cpp
)

# -Wall -> be strict with warnings
set(MCMINI_EXTRA_COMPILER_FLAGS -Wall)
set(MCMINI_EXTRA_COMPILER_DEFINITIONS MC_SHARED_LIBRARY=1)

# -lrt -> shared memory
# -pthread -> libpthread.so
# -lm -> math library
# -ldl -> dlsym etc.
set(MCMINI_EXTRA_LINK_FLAGS -lrt -pthread -lm -ldl)

# libmcmini.so -> the dylib which is loaded
add_library(libmcmini SHARED "${LIBMCMINI_CPP_SRC}""${LIBMCMINI_C_SRC}")
add_library(McMini::Dylib ALIAS libmcmini)
set_target_properties(libmcmini PROPERTIES OUTPUT_NAME "mcmini")

target_include_directories(libmcmini PUBLIC "${MCMINI_INCLUDE_DIR}")
target_compile_definitions(libmcmini
  PUBLIC
    "${MCMINI_EXTRA_COMPILER_DEFINITIONS}")
target_compile_options(libmcmini
  PRIVATE
    "${MCMINI_EXTRA_COMPILER_FLAGS}")
target_link_libraries(libmcmini
  PUBLIC
    "${MCMINI_EXTRA_LINK_FLAGS}")

add_executable(mcmini "${MCMINI_CPP_SRC}""${MCMINI_C_SRC}")
target_include_directories(mcmini PUBLIC "${MCMINI_INCLUDE_DIR}")

add_executable(api-demo src/demos/transition-api-demo.cpp)