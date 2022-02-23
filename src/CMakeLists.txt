cmake_minimum_required(VERSION 3.21)
project(DPOR C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(DPOR
        array.c
        array.h
        dpor.c
        dpor.h
        mutex.c
        mutex.h
        philosphers-deadlock.c
        pthread_wrappers.c
        pthread_wrappers.h
        shared_state.c
        shared_state.h
        simple-deadlock.c
        thread.c
        thread.h
        transition.c
        transition.h
        visible_operation.c
        visible_operation.h
        mc_shared_cv.c
        mc_shared_cv.h
        shm.c
        shm.h fail.c fail.h)
