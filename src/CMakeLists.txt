
include_directories(.)

message("${BUILD_TESTS} is on!jj")
set(MAIN_FUNCTION main.cpp)
set(MC_C_FILES mc_shared_cv.c mc_shared_cv.h
        transitions/wrappers/MCSharedLibraryWrappers.c transitions/wrappers/MCSharedLibraryWrappers.h)
set(MC_CPP_FILES
        MCShared.h
        objects/MCThread.cpp objects/MCThread.h
        objects/MCVisibleObject.cpp objects/MCVisibleObject.h
        MCObjectStore.cpp MCObjectStore.h
        MCSharedTransition.cpp MCSharedTransition.h
        MCMINIWrappers.h
        MCTransition.cpp MCTransition.h
        MCMINI_Private.cpp MCMINI_Private.h
        MCState.cpp MCState.h MC_Private.h MCConstants.h
        transitions/threads/MCThreadStart.cpp transitions/threads/MCThreadStart.h
        MCTransitionFactory.cpp MCTransitionFactory.h
        transitions/wrappers/MCThreadTransitionWrappers.cpp
        transitions/wrappers/MCThreadTransitionWrappers.h
        MCStateStackItem.cpp MCStateStackItem.h
        transitions/threads/MCThreadCreate.cpp transitions/threads/MCThreadCreate.h
        transitions/threads/MCThreadTransition.cpp transitions/threads/MCThreadTransition.h
        transitions/threads/MCThreadFinish.cpp transitions/threads/MCThreadFinish.h
        transitions/threads/MCThreadJoin.cpp transitions/threads/MCThreadJoin.h
        objects/MCMutex.cpp objects/MCMutex.h
        transitions/mutex/MCMutexTransition.cpp transitions/mutex/MCMutexTransition.h
        transitions/mutex/MCMutexLock.cpp transitions/mutex/MCMutexLock.h
        transitions/mutex/MCMutexInit.cpp transitions/mutex/MCMutexInit.h
        transitions/mutex/MCMutexUnlock.cpp transitions/mutex/MCMutexUnlock.h
        transitions/wrappers/MCMutexTransitionWrappers.cpp transitions/wrappers/MCMutexTransitionWrappers.h
        transitions/MCTransitionsShared.cpp transitions/MCTransitionsShared.h
        objects/MCSemaphore.cpp objects/MCSemaphore.h
        transitions/semaphore/MCSemaphoreTransition.cpp transitions/semaphore/MCSemaphoreTransition.h
        transitions/semaphore/MCSemPost.cpp transitions/semaphore/MCSemPost.h
        transitions/semaphore/MCSemInit.cpp transitions/semaphore/MCSemInit.h
        transitions/semaphore/MCSemWait.cpp transitions/semaphore/MCSemWait.h
        transitions/wrappers/MCSemaphoreTransitionWrappers.cpp
        transitions/wrappers/MCSemaphoreTransitionWrappers.h
        MCDeferred.h transitions/misc/MCExitTransition.cpp
        transitions/misc/MCExitTransition.h MCStateConfiguration.cpp
        objects/MCGlobalVariable.cpp objects/MCGlobalVariable.h
        MCStateConfiguration.h objects/MCBarrier.cpp objects/MCBarrier.h
        transitions/barrier/MCBarrierInit.cpp transitions/barrier/MCBarrierInit.h
        transitions/barrier/MCBarrierWait.cpp transitions/barrier/MCBarrierWait.h
        transitions/barrier/MCBarrierTransition.cpp
        transitions/barrier/MCBarrierTransition.h
        transitions/wrappers/MCBarrierWrappers.cpp
        transitions/wrappers/MCBarrierWrappers.h transitions/wrappers/MCConditionVariableWrappers.cpp
        transitions/wrappers/MCConditionVariableWrappers.h objects/MCConditionVariable.cpp
        objects/MCConditionVariable.h transitions/cond/MCCondInit.cpp
        transitions/cond/MCCondInit.h transitions/cond/MCCondTransition.cpp
        transitions/cond/MCCondTransition.h transitions/cond/MCCondWait.cpp
        transitions/cond/MCCondWait.h misc/MCOptional.h
        transitions/cond/MCCondBroadcast.cpp transitions/cond/MCCondBroadcast.h
        transitions/cond/MCCondSignal.cpp transitions/cond/MCCondSignal.h
        transitions/cond/MCCondEnqueue.cpp
        transitions/cond/MCCondEnqueue.h
        transitions/semaphore/MCSemEnqueue.cpp
        transitions/semaphore/MCSemEnqueue.h
        transitions/barrier/MCBarrierEnqueue.cpp
        transitions/barrier/MCBarrierEnqueue.h
        transitions/misc/MCGlobalVariableRead.cpp
        transitions/misc/MCGlobalVariableWrite.cpp
        transitions/threads/MCThreadReachGoal.cpp
        transitions/wrappers/MCGlobalVariableWrappers.cpp
        MCCommon.c MCCommon.h
        MCEnv.h transitions/threads/MCThreadEnterGoalCriticalSection.cpp 
				transitions/threads/MCThreadEnterGoalCriticalSection.h
				transitions/threads/MCThreadExitGoalCriticalSection.cpp 
				transitions/threads/MCThreadExitGoalCriticalSection.h
        MCMINI.h)

add_executable(MCMINI ${MC_CPP_FILES} ${MC_C_FILES} ${MAIN_FUNCTION})
message("Linking object files...")

# -lrt -> shared memory
# -pthread -> libpthread.so
# -lm -> math library
# -ldl -> dlsym etc.
set(MC_LINK_TARGET_FLAGS -lrt -pthread -lm -ldl)
target_link_libraries(MCMINI PUBLIC ${MC_LINK_TARGET_FLAGS})

# -g3 -> debug symbols
#-fsanitize=undefined (-lubsan)
target_compile_options(MCMINI PUBLIC -O3)

# Shared library
add_library(mcminichecker SHARED ${MC_CPP_FILES} ${MC_C_FILES} ${MAIN_FUNCTION})
target_compile_definitions(mcminichecker PUBLIC MC_SHARED_LIBRARY=1)
target_link_libraries(mcminichecker PUBLIC ${MC_LINK_TARGET_FLAGS})
target_compile_options(mcminichecker PUBLIC -O3)

# Executable that loads the shared library
set(MAIN_FUNCTION launch.c)
add_executable(mcmini ${MAIN_FUNCTION} MCEnv.h)
target_compile_options(mcmini PUBLIC -O3)
