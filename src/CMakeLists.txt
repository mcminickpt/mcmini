cmake_minimum_required(VERSION 2.8)
project(GMAL C CXX)
message("***** GMAL MODEL CHECKER *****")

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
        GMALWrappers.cpp GMALWrappers.h
        GMALTransition.cpp GMALTransition.h
        GMAL.cpp GMAL.h
        GMALState.cpp GMALState.h GMAL_Private.h GMALConstants.h
        transitions/GMALThreadStart.cpp transitions/GMALThreadStart.h
        GMALTransitionFactory.cpp GMALTransitionFactory.h
        transitions/wrappers/GMALThreadTransitionWrappers.cpp
        transitions/wrappers/GMALThreadTransitionWrappers.h
        GMALStateStackItem.cpp GMALStateStackItem.h
        transitions/GMALThreadCreate.cpp transitions/GMALThreadCreate.h
        transitions/GMALThreadTransition.cpp transitions/GMALThreadTransition.h
        transitions/GMALThreadFinish.cpp transitions/GMALThreadFinish.h
        transitions/GMALThreadJoin.cpp transitions/GMALThreadJoin.h
        objects/GMALMutex.cpp objects/GMALMutex.h
        transitions/GMALMutexTransition.cpp transitions/GMALMutexTransition.h
        transitions/GMALMutexLock.cpp transitions/GMALMutexLock.h
        transitions/GMALMutexInit.cpp transitions/GMALMutexInit.h
        transitions/GMALMutexUnlock.cpp transitions/GMALMutexUnlock.h
        transitions/wrappers/GMALMutexTransitionWrappers.cpp transitions/wrappers/GMALMutexTransitionWrappers.h
        transitions/GMALTransitionsShared.cpp transitions/GMALTransitionsShared.h objects/GMALSemaphore.cpp objects/GMALSemaphore.h transitions/GMALSemaphoreTransition.cpp transitions/GMALSemaphoreTransition.h transitions/GMALSemPost.cpp transitions/GMALSemPost.h transitions/GMALSemInit.cpp transitions/GMALSemInit.h transitions/GMALSemWait.cpp transitions/GMALSemWait.h transitions/wrappers/GMALSemaphoreTransitionWrappers.cpp transitions/wrappers/GMALSemaphoreTransitionWrappers.h GMALDeferred.h transitions/GMALExitTransition.cpp transitions/GMALExitTransition.h GMALStateConfiguration.cpp GMALStateConfiguration.h objects/GMALBarrier.cpp objects/GMALBarrier.h transitions/GMALBarrierInit.cpp transitions/GMALBarrierInit.h transitions/GMALBarrierWait.cpp transitions/GMALBarrierWait.h transitions/GMALBarrierTransition.cpp transitions/GMALBarrierTransition.h transitions/wrappers/GMALBarrierWrappers.cpp transitions/wrappers/GMALBarrierWrappers.h transitions/wrappers/GMALConditionVariableWrappers.cpp transitions/wrappers/GMALConditionVariableWrappers.h objects/GMALConditionVariable.cpp objects/GMALConditionVariable.h transitions/GMALCondInit.cpp transitions/GMALCondInit.h transitions/GMALCondTransition.cpp transitions/GMALCondTransition.h transitions/GMALCondWait.cpp transitions/GMALCondWait.h misc/GMALOptional.h transitions/GMALCondBroadcast.cpp transitions/GMALCondBroadcast.h transitions/GMALCondSignal.cpp transitions/GMALCondSignal.h)

add_executable(GMAL ${GMAL_CPP_FILES} ${GMAL_C_FILES} ${MAIN_FUNCTION})
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
set(MAIN_FUNCTION launch.c)
add_executable(gmal ${MAIN_FUNCTION})
target_compile_options(gmal PUBLIC -g3)