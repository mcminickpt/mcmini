cmake_minimum_required(VERSION 3.21)
project(DPOR C)
message("*****DPOR PROJECT*****")

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(DPOR
        array.c array.h
        common.c common.h
        decl.h
        dpor.c dpor.h
        fail.c fail.h
        hashtable.c hashtable.h
        mc_shared_cv.c mc_shared_cv.h
        mutex.c mutex.h
        philosphers-deadlock.c
        pthread_wrappers.c pthread_wrappers.h
        shared_state.c shared_state.h
        state_stack_item.c state_stack_item.h
        shm.c shm.h
        simple-deadlock.c
        thread.c thread.h
        transition.c transition.h
        visible_operation.c visible_operation.h)
message("Linking object files...")
target_link_libraries(DPOR PUBLIC -lrt -pthread)