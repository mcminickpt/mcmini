cmake_minimum_required(VERSION 3.10)
project(McMini-Revamp
    VERSION 1.0.0
    DESCRIPTION "A bite-sized model checker"
    LANGUAGES C CXX)

# Require C11 and C++11
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


# Project configuration
option(BUILD_TESTS OFF)
option(VERBOSE_TESTING OFF)
set(MCMINI_DIR "${CMAKE_SOURCE_DIR}")
set(MCMINI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(MCMINI_CMAKE_MODULE_DIR "${CMAKE_SOURCE_DIR}/cmake")
set(MCMINI_WITH_DMTCP OFF)

# Project source files
set(MCMINI_C_SRC
  src/common/exit.c
  src/common/mem.c
  src/common/runner_mailbox.c
  src/common/shm_config.c
)
set(MCMINI_CPP_SRC
  src/mcmini/mcmini.cpp
  src/mcmini/signal.cpp
  src/mcmini/visible_object.cpp
  src/mcmini/coordinator/coordinator.cpp
  src/mcmini/model/detached_state.cpp
  src/mcmini/model/diff_state.cpp
  src/mcmini/model/program.cpp
  src/mcmini/model/cond_var_arbitrary_policy.cpp
  src/mcmini/model/cond_var_default_policy.cpp
  src/mcmini/model/cond_var_single_grp_policy.cpp
  src/mcmini/model/cond_var_wakegroup.cpp
  src/mcmini/model/state_sequence.cpp
  src/mcmini/model/transition_registry.cpp
  src/mcmini/model/transition_sequence.cpp
  src/mcmini/model/transitions/mutex.cpp
  src/mcmini/model/transitions/condition_variables.cpp
  src/mcmini/model/transitions/thread.cpp
  src/mcmini/model/transitions/semaphore.cpp
  src/mcmini/model_checking/algorithms/classic_dpor.cpp
  src/mcmini/model_checking/algorithms/clock_vector.cpp
  src/mcmini/real_world/dmtcp_process_source.cpp
  src/mcmini/real_world/fifo.cpp
  src/mcmini/real_world/fork_process_source.cpp
  src/mcmini/real_world/local_linux_process.cpp
  src/mcmini/real_world/resources.cpp
  src/mcmini/real_world/shm.cpp
  src/mcmini/real_world/target.cpp
)
set(LIBMCMINI_C_SRC
  src/common/exit.c
  src/common/mem.c
  src/common/multithreaded_fork.c
  src/common/runner_mailbox.c
  src/common/shm_config.c
  src/lib/dmtcp-callback.c
  src/lib/entry.c
  src/lib/interception.c
  src/lib/log.c
  src/lib/main.c
  src/lib/record.c
  src/lib/sem-wrappers.c
  src/lib/template/loop.c
  src/lib/template/sig.c
  src/lib/wrappers.c
  src/mcmini/Thread_queue.c
)

set(MCMINI_EXTRA_COMPILER_FLAGS -Wall -Werror)
set(MCMINI_EXTRA_COMPILER_DEFINITIONS "")
set(MCMINI_EXTRA_LINK_FLAGS -lrt -pthread)

set(LIBMCMINI_EXTRA_COMPILER_FLAGS -Wall -Werror -fPIC)
set(LIBMCMINI_EXTRA_COMPILER_DEFINITIONS MC_SHARED_LIBRARY=1 DMTCP=1 LOGGING_ROOT="${CMAKE_SOURCE_PATH}")

# -lrt -> shared memory
# -pthread -> libpthread.so
# -lm -> math library
# -ldl -> dlsym etc.
set(LIBMCMINI_EXTRA_LINK_FLAGS -lrt -pthread -lm -ldl)

# libmcmini.so -> the dylib which is loaded
add_library(libmcmini SHARED "${LIBMCMINI_C_SRC}")
set_target_properties(libmcmini PROPERTIES OUTPUT_NAME "mcmini")

target_include_directories(libmcmini PUBLIC "${MCMINI_INCLUDE_DIR}")
target_compile_definitions(libmcmini
  PUBLIC
    "${LIBMCMINI_EXTRA_COMPILER_DEFINITIONS}")
target_compile_options(libmcmini
  PRIVATE
    "${LIBMCMINI_EXTRA_COMPILER_FLAGS}")
target_link_libraries(libmcmini
  PUBLIC
    "${LIBMCMINI_EXTRA_LINK_FLAGS}")

add_executable(mcmini "${MCMINI_CPP_SRC}" "${MCMINI_C_SRC}")
target_include_directories(mcmini PUBLIC "${MCMINI_INCLUDE_DIR}")
target_compile_definitions(mcmini
  PUBLIC
    "${MCMINI_EXTRA_COMPILER_DEFINITIONS}")
target_compile_options(mcmini
  PRIVATE
    "${MCMINI_EXTRA_COMPILER_FLAGS}")
target_link_libraries(mcmini
  PUBLIC
    "${MCMINI_EXTRA_LINK_FLAGS}")

add_subdirectory(src/examples)
