cmake_minimum_required(VERSION 3.21)
project(DPOR C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(DPOR
        src/array.c
        src/array.h
        src/mutex.c
        src/mutex.h
        src/philosphers-deadlock.c
        src/pthread_wrappers.c
        src/pthread_wrappers.h
        src/shared_state.c
        src/shared_state.h
        src/simple-deadlock.c
        src/thread.c
        src/thread.h
        src/transition.c
        src/transition.h
        src/visible_operation.c
        src/visible_operation.h src/cooplock.c src/cooplock.h src/shm.c src/shm.h)
