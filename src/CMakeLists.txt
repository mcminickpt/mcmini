cmake_minimum_required(VERSION 2.8)
project(MCMINI C CXX)
message("***** MCMINI MODEL CHECKER *****")

set(CMAKE_GENERATOR "Unix Makefiles")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

include_directories(.)
set(MAIN_FUNCTION main.cpp)
set(MC_C_FILES mc_shared_cv.c mc_shared_cv.h
        transitions/wrappers/MCSharedLibraryWrappers.c transitions/wrappers/MCSharedLibraryWrappers.h)
set(MC_CPP_FILES
        MCShared.h
        objects/MCThread.cpp objects/MCThread.h
        objects/MCVisibleObject.cpp objects/MCVisibleObject.h
        MCObjectStore.cpp MCObjectStore.h
        MCSharedTransition.cpp MCSharedTransition.h
        MCMINIWrappers.cpp MCMINIWrappers.h
        MCTransition.cpp MCTransition.h
        MCMINI.cpp MCMINI.h
        MCState.cpp MCState.h MC_Private.h MCConstants.h
        transitions/MCThreadStart.cpp transitions/MCThreadStart.h
        MCTransitionFactory.cpp MCTransitionFactory.h
        transitions/wrappers/MCThreadTransitionWrappers.cpp
        transitions/wrappers/MCThreadTransitionWrappers.h
        MCStateStackItem.cpp MCStateStackItem.h
        transitions/MCThreadCreate.cpp transitions/MCThreadCreate.h
        transitions/MCThreadTransition.cpp transitions/MCThreadTransition.h
        transitions/MCThreadFinish.cpp transitions/MCThreadFinish.h
        transitions/MCThreadJoin.cpp transitions/MCThreadJoin.h
        objects/MCMutex.cpp objects/MCMutex.h
        transitions/MCMutexTransition.cpp transitions/MCMutexTransition.h
        transitions/MCMutexLock.cpp transitions/MCMutexLock.h
        transitions/MCMutexInit.cpp transitions/MCMutexInit.h
        transitions/MCMutexUnlock.cpp transitions/MCMutexUnlock.h
        transitions/wrappers/MCMutexTransitionWrappers.cpp transitions/wrappers/MCMutexTransitionWrappers.h
        transitions/MCTransitionsShared.cpp transitions/MCTransitionsShared.h objects/MCSemaphore.cpp objects/MCSemaphore.h transitions/MCSemaphoreTransition.cpp transitions/MCSemaphoreTransition.h transitions/MCSemPost.cpp transitions/MCSemPost.h transitions/MCSemInit.cpp transitions/MCSemInit.h transitions/MCSemWait.cpp transitions/MCSemWait.h transitions/wrappers/MCSemaphoreTransitionWrappers.cpp transitions/wrappers/MCSemaphoreTransitionWrappers.h MCDeferred.h transitions/MCExitTransition.cpp transitions/MCExitTransition.h MCStateConfiguration.cpp MCStateConfiguration.h objects/MCBarrier.cpp objects/MCBarrier.h transitions/MCBarrierInit.cpp transitions/MCBarrierInit.h transitions/MCBarrierWait.cpp transitions/MCBarrierWait.h transitions/MCBarrierTransition.cpp transitions/MCBarrierTransition.h transitions/wrappers/MCBarrierWrappers.cpp transitions/wrappers/MCBarrierWrappers.h transitions/wrappers/MCConditionVariableWrappers.cpp transitions/wrappers/MCConditionVariableWrappers.h objects/MCConditionVariable.cpp objects/MCConditionVariable.h transitions/MCCondInit.cpp transitions/MCCondInit.h transitions/MCCondTransition.cpp transitions/MCCondTransition.h transitions/MCCondWait.cpp transitions/MCCondWait.h misc/MCOptional.h transitions/MCCondBroadcast.cpp transitions/MCCondBroadcast.h transitions/MCCondSignal.cpp transitions/MCCondSignal.h transitions/MCCondEnqueue.cpp transitions/MCCondEnqueue.h MCEnv.h)

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
target_compile_options(MCMINI PUBLIC -g3)

# Shared library
add_library(gmalchecker SHARED ${MC_CPP_FILES} ${MC_C_FILES} ${MAIN_FUNCTION})
target_compile_definitions(gmalchecker PUBLIC MC_SHARED_LIBRARY=1)
target_link_libraries(gmalchecker PUBLIC ${MC_LINK_TARGET_FLAGS})
target_compile_options(gmalchecker PUBLIC -g3)

# Executable that loads the shared library
set(MAIN_FUNCTION launch.c ../test/programs/producer_consumer.cpp)
<<<<<<< HEAD
add_executable(gmal ${MAIN_FUNCTION} MCEnv.h)
=======
add_executable(gmal ${MAIN_FUNCTION} MCEnv.h)
>>>>>>> Symbols:  MC->MC
target_compile_options(gmal PUBLIC -g3)
