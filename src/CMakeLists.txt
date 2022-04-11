cmake_minimum_required(VERSION 3.21)
project(GMAL C CXX)
message("***** GMAL MODEL CHECKER *****")

set(CMAKE_CXX_STANDARD 11)

include_directories(.)
#add_executable(GMAL
#        array.c array.h
#        common.c common.h
#        decl.h
#        dpor.c dpor.h
#        fail.c fail.h
#        hashtable.c hashtable.h
#        mc_shared_cv.c mc_shared_cv.h
#        mutex.c mutex.h
#        philosphers-deadlock.c
#        pthread_wrappers.c pthread_wrappers.h
#        shared_state.c shared_state.h
#        state_stack_item.c state_stack_item.h
#        shm.c shm.h
#        simple-deadlock.cpp
#        thread.c thread.h
#        transition.c transition.h
#        visible_operation.c visible_operation.h
#        concurrent_system.c concurrent_system.h
#        hashset.c hashset.h stats.h
#        GMALShared.h GMALRef.cpp GMALRef.h
#        GMALMap.cpp GMALMap.h
#        GMALThread.cpp GMALThread.h
#        GMALVisibleObject.cpp GMALVisibleObject.h
#        GMALObjectStore.cpp GMALObjectStore.h
#        GMALSharedTransition.cpp GMALSharedTransition.h main.cpp)
set (GMAL_C_FILES mc_shared_cv.c mc_shared_cv.h)
add_executable(GMAL
        GMALShared.h GMALRef.cpp GMALRef.h
        GMALMap.cpp GMALMap.h
        GMALThread.cpp GMALThread.h
        GMALVisibleObject.cpp GMALVisibleObject.h
        GMALObjectStore.cpp GMALObjectStore.h
        GMALSharedTransition.cpp GMALSharedTransition.h main.cpp
        GMALWrappers.cpp GMALWrappers.h
        GMALTransition.cpp GMALTransition.h
        GMAL.cpp GMAL.h
        GMALState.cpp GMALState.h GMAL_Private.h GMALConstants.h
        ${GMAL_C_FILES} GMALThreadStart.cpp GMALThreadStart.h GMALTransitionFactory.cpp GMALTransitionFactory.h)
message("Linking object files...")


# -lrt -> shared memory
# -pthread -> libpthread.so
# -lm -> math library
#target_compile_options(DPOR PUBLIC -fsanitize=undefined) -lubsan
set(GMAL_LINK_TARGET_FLAGS -lrt -pthread -lm)
target_link_libraries(GMAL PUBLIC ${GMAL_LINK_TARGET_FLAGS})

