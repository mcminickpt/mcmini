cmake_minimum_required(VERSION 2.8)
project(GMAL C CXX)
message("***** GMAL MODEL CHECKER *****")

set(CMAKE_GENERATOR "Unix Makefiles")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

include_directories(.)
set(MAIN_FUNCTION main.cpp)
set(GMAL_C_FILES mc_shared_cv.c mc_shared_cv.h
        transitions/wrappers/GMALSharedLibraryWrappers.c transitions/wrappers/GMALSharedLibraryWrappers.h)
set(GMAL_CPP_FILES
        GMALShared.h
        objects/GMALThread.cpp objects/GMALThread.h
        objects/GMALVisibleObject.cpp objects/GMALVisibleObject.h
        GMALObjectStore.cpp GMALObjectStore.h
        GMALSharedTransition.cpp GMALSharedTransition.h
        GMALWrappers.h
        GMALTransition.cpp GMALTransition.h
        GMAL.cpp GMAL.h
        GMALState.cpp GMALState.h GMAL_Private.h GMALConstants.h
        transitions/threads/GMALThreadStart.cpp transitions/threads/GMALThreadStart.h
        GMALTransitionFactory.cpp GMALTransitionFactory.h
        transitions/wrappers/GMALThreadTransitionWrappers.cpp
        transitions/wrappers/GMALThreadTransitionWrappers.h
        GMALStateStackItem.cpp GMALStateStackItem.h
        transitions/threads/GMALThreadCreate.cpp transitions/threads/GMALThreadCreate.h
        transitions/threads/GMALThreadTransition.h
        transitions/threads/GMALThreadFinish.cpp transitions/threads/GMALThreadFinish.h
        transitions/threads/GMALThreadJoin.cpp transitions/threads/GMALThreadJoin.h
        objects/GMALMutex.cpp objects/GMALMutex.h
        transitions/mutex/GMALMutexTransition.h
        transitions/mutex/GMALMutexLock.cpp transitions/mutex/GMALMutexLock.h
        transitions/mutex/GMALMutexInit.cpp transitions/mutex/GMALMutexInit.h
        transitions/mutex/GMALMutexUnlock.cpp transitions/mutex/GMALMutexUnlock.h
        transitions/wrappers/GMALMutexTransitionWrappers.cpp transitions/wrappers/GMALMutexTransitionWrappers.h
        transitions/GMALTransitionsShared.cpp transitions/GMALTransitionsShared.h
        objects/GMALSemaphore.cpp objects/GMALSemaphore.h
        transitions/semaphore/GMALSemaphoreTransition.h
        transitions/semaphore/GMALSemPost.cpp transitions/semaphore/GMALSemPost.h
        transitions/semaphore/GMALSemInit.cpp transitions/semaphore/GMALSemInit.h
        transitions/semaphore/GMALSemWait.cpp transitions/semaphore/GMALSemWait.h
        transitions/wrappers/GMALSemaphoreTransitionWrappers.cpp
        transitions/wrappers/GMALSemaphoreTransitionWrappers.h
        GMALDeferred.h
        transitions/misc/GMALExitTransition.cpp transitions/misc/GMALExitTransition.h
        GMALStateConfiguration.cpp GMALStateConfiguration.h
        objects/GMALBarrier.cpp objects/GMALBarrier.h
        transitions/barrier/GMALBarrierInit.cpp transitions/barrier/GMALBarrierInit.h
        transitions/barrier/GMALBarrierWait.cpp transitions/barrier/GMALBarrierWait.h
        transitions/barrier/GMALBarrierTransition.h
        transitions/wrappers/GMALBarrierWrappers.cpp transitions/wrappers/GMALBarrierWrappers.h
        transitions/wrappers/GMALConditionVariableWrappers.cpp transitions/wrappers/GMALConditionVariableWrappers.h
        objects/GMALConditionVariable.cpp objects/GMALConditionVariable.h
        transitions/cond/GMALCondInit.cpp transitions/cond/GMALCondInit.h
        transitions/cond/GMALCondTransition.h
        transitions/cond/GMALCondWait.cpp transitions/cond/GMALCondWait.h misc/GMALOptional.h
        transitions/cond/GMALCondBroadcast.cpp transitions/cond/GMALCondBroadcast.h
        transitions/cond/GMALCondSignal.cpp transitions/cond/GMALCondSignal.h
        transitions/cond/GMALCondEnqueue.cpp transitions/cond/GMALCondEnqueue.h
        GMALEnv.h objects/GMALGlobalVariable.cpp objects/GMALGlobalVariable.h
        transitions/barrier/GMALBarrierEnqueue.cpp transitions/barrier/GMALBarrierEnqueue.h
        transitions/misc/GMALGlobalVariableTransition.h
        transitions/misc/GMALGlobalVariableRead.cpp transitions/misc/GMALGlobalVariableRead.h
        transitions/misc/GMALGlobalVariableWrite.cpp transitions/misc/GMALGlobalVariableWrite.h
        transitions/wrappers/GMALGlobalVariableWrappers.cpp
        transitions/wrappers/GMALGlobalVariableWrappers.h
        transitions/semaphore/GMALSemEnqueue.cpp transitions/semaphore/GMALSemEnqueue.h transitions/threads/GMALThreadReachGoal.cpp transitions/threads/GMALThreadReachGoal.h transitions/barrier/GMALBarrierDefs.h transitions/threads/GMALThreadDefs.h transitions/semaphore/GMALSemaphoreDefs.h transitions/mutex/GMALMutexDefs.h transitions/cond/GMALCondDefs.h transitions/misc/GMALMiscDefs.h)

add_executable(GMAL ${GMAL_CPP_FILES} ${GMAL_C_FILES} ${MAIN_FUNCTION} transitions/semaphore/GMALSemEnqueue.cpp transitions/semaphore/GMALSemEnqueue.h)
message("Linking object files...")

# -lrt -> shared memory
# -pthread -> libpthread.so
# -lm -> math library
# -ldl -> dlsym etc.
set(GMAL_LINK_TARGET_FLAGS -lrt -pthread -lm -ldl)
target_link_libraries(GMAL PUBLIC ${GMAL_LINK_TARGET_FLAGS})

# -g3 -> debug symbols
#-fsanitize=undefined (-lubsan)
target_compile_options(GMAL PUBLIC -g3)

# Shared library
add_library(gmalchecker SHARED ${GMAL_CPP_FILES} ${GMAL_C_FILES} ${MAIN_FUNCTION})
target_compile_definitions(gmalchecker PUBLIC GMAL_SHARED_LIBRARY=1)
target_link_libraries(gmalchecker PUBLIC ${GMAL_LINK_TARGET_FLAGS})
target_compile_options(gmalchecker PUBLIC -g3)

# Executable that loads the shared library
set(MAIN_FUNCTION launch.c ../test/programs/producer_consumer.cpp)
add_executable(gmal ${MAIN_FUNCTION} GMALEnv.h)
target_compile_options(gmal PUBLIC -g3)