set(MAIN_FUNCTION main.cpp)
set(MC_C_FILES
  mc_shared_cv.c
  MCCommon.c
  transitions/wrappers/MCSharedLibraryWrappers.c
)

set(MC_CPP_FILES
  MCObjectStore.cpp
  MCSharedTransition.cpp
  MCTransition.cpp
  MCState.cpp
  MCTransitionFactory.cpp
  MCStateStackItem.cpp
  MCThreadData.cpp
  MCClockVector.cpp
  mcmini_private.cpp
  signals.cpp

  misc/cond/MCCondVarWakeupPolicyImpl.cpp
  misc/cond/MCCondVarArbitraryWakeupPolicy.cpp
  misc/cond/MCCondVarOrderedWakeupPolicy.cpp
  misc/cond/MCCondVarGLibcSignalPolicy.cpp

  misc/cond/MCCondVarSingleGroupSignalPolicy.cpp
  misc/cond/MCWakeGroup.cpp

  objects/MCThread.cpp
  objects/MCVisibleObject.cpp
  objects/MCMutex.cpp
  objects/MCRWLock.cpp
  objects/MCRWWLock.cpp
  objects/MCSemaphore.cpp
  objects/MCGlobalVariable.cpp
  objects/MCBarrier.cpp
  objects/MCConditionVariable.cpp

  transitions/barrier/MCBarrierEnqueue.cpp
  transitions/barrier/MCBarrierInit.cpp
  transitions/barrier/MCBarrierWait.cpp
  transitions/cond/MCCondBroadcast.cpp
  transitions/cond/MCCondEnqueue.cpp
  transitions/cond/MCCondInit.cpp
  transitions/cond/MCCondSignal.cpp
  transitions/cond/MCCondWait.cpp
  transitions/MCTransitionsShared.cpp
  transitions/misc/MCExitTransition.cpp
  transitions/misc/MCAbortTransition.cpp
  transitions/misc/MCGlobalVariableRead.cpp
  transitions/misc/MCGlobalVariableWrite.cpp
  transitions/mutex/MCMutexInit.cpp
  transitions/mutex/MCMutexLock.cpp
  transitions/mutex/MCMutexUnlock.cpp
  transitions/semaphore/MCSemEnqueue.cpp
  transitions/semaphore/MCSemInit.cpp
  transitions/semaphore/MCSemPost.cpp
  transitions/semaphore/MCSemWait.cpp
  transitions/threads/MCThreadCreate.cpp
  transitions/threads/MCThreadFinish.cpp
  transitions/threads/MCThreadJoin.cpp
  transitions/threads/MCThreadStart.cpp
  transitions/rwlock/MCRWLockInit.cpp
  transitions/rwlock/MCRWLockReaderLock.cpp
  transitions/rwlock/MCRWLockWriterLock.cpp
  transitions/rwlock/MCRWLockUnlock.cpp
  transitions/rwlock/MCRWLockWriterEnqueue.cpp
  transitions/rwlock/MCRWLockReaderEnqueue.cpp
  transitions/rwwlock/MCRWWLockInit.cpp
  transitions/rwwlock/MCRWWLockReaderEnqueue.cpp
  transitions/rwwlock/MCRWWLockReaderLock.cpp
  transitions/rwwlock/MCRWWLockWriter1Enqueue.cpp
  transitions/rwwlock/MCRWWLockWriter1Lock.cpp
  transitions/rwwlock/MCRWWLockWriter2Enqueue.cpp
  transitions/rwwlock/MCRWWLockWriter2Lock.cpp
  transitions/rwwlock/MCRWWLockUnlock.cpp
  transitions/wrappers/MCBarrierWrappers.cpp
  transitions/wrappers/MCConditionVariableWrappers.cpp
  transitions/wrappers/MCGlobalVariableWrappers.cpp
  transitions/wrappers/MCMutexTransitionWrappers.cpp
  transitions/wrappers/MCSemaphoreTransitionWrappers.cpp
  transitions/wrappers/MCSharedLibraryWrappers.c
  transitions/wrappers/MCThreadTransitionWrappers.cpp
  transitions/wrappers/MCRWLockWrappers.cpp
  transitions/wrappers/MCRWWLockWrappers.cpp
)

set(MC_COMPILER_EXPLICIT_COMPILER_FLAGS -Wall)
# -lrt -> shared memory
# -pthread -> libpthread.so
# -lm -> math library
# -ldl -> dlsym etc.
set(MC_LINK_TARGET_FLAGS -lrt -pthread -lm -ldl)

add_executable(mcmini_demo ${MC_CPP_FILES} ${MC_C_FILES} ${MAIN_FUNCTION})
add_executable(McMini::Demo ALIAS mcmini_demo)
set_target_properties(mcmini_demo PROPERTIES OUTPUT_NAME "mcmini-demo")
target_include_directories(mcmini_demo PUBLIC "${MCMINI_INCLUDE_DIR}")
target_link_libraries(mcmini_demo
  PUBLIC
    ${MC_LINK_TARGET_FLAGS})


add_library(mcmini_checker SHARED ${MC_CPP_FILES} ${MC_C_FILES} ${MAIN_FUNCTION})
add_library(McMini::Dylib ALIAS mcmini_checker)
set_target_properties(mcmini_checker PROPERTIES OUTPUT_NAME "mcmini")
target_include_directories(mcmini_checker PUBLIC "${MCMINI_INCLUDE_DIR}")
target_compile_definitions(mcmini_checker
  PUBLIC
    MC_SHARED_LIBRARY=1)
target_compile_options(mcmini_checker
  PRIVATE
    ${MC_COMPILER_EXPLICIT_COMPILER_FLAGS})
target_link_libraries(mcmini_checker
  PUBLIC
    ${MC_LINK_TARGET_FLAGS})

# Executable that loads the shared library
set(MAIN_FUNCTION launch.c)
add_executable(mcmini ${MAIN_FUNCTION})
target_include_directories(mcmini PUBLIC "${MCMINI_INCLUDE_DIR}")
target_compile_options(mcmini PUBLIC)

# Exported test libraries
add_library(mcrwlock_lib STATIC export/rwwlock.c)
add_library(McMini::RWWLockLib ALIAS mcrwlock_lib)
target_include_directories(mcrwlock_lib PUBLIC "${MCMINI_INCLUDE_DIR}")

# For the demo, we add the
target_link_libraries(mcmini_demo PUBLIC McMini::RWWLockLib)
