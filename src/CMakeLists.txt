cmake_minimum_required(VERSION 3.21)
project(GMAL C CXX)
message("***** GMAL MODEL CHECKER *****")

set(CMAKE_CXX_STANDARD 11)

include_directories(.)
set (GMAL_C_FILES mc_shared_cv.c mc_shared_cv.h)
add_executable(GMAL
        GMALShared.h
        GMALMap.cpp GMALMap.h
        GMALThread.cpp GMALThread.h
        GMALVisibleObject.cpp GMALVisibleObject.h
        GMALObjectStore.cpp GMALObjectStore.h
        GMALSharedTransition.cpp GMALSharedTransition.h main.cpp
        GMALWrappers.cpp GMALWrappers.h
        GMALTransition.cpp GMALTransition.h
        GMAL.cpp GMAL.h
        GMALState.cpp GMALState.h GMAL_Private.h GMALConstants.h
        ${GMAL_C_FILES} transitions/GMALThreadStart.cpp transitions/GMALThreadStart.h GMALTransitionFactory.cpp GMALTransitionFactory.h transitions/GMALThreadTransitionWrappers.cpp transitions/GMALThreadTransitionWrappers.h)
message("Linking object files...")

# -lrt -> shared memory
# -pthread -> libpthread.so
# -lm -> math library
#target_compile_options(DPOR PUBLIC -fsanitize=undefined) -lubsan
set(GMAL_LINK_TARGET_FLAGS -lrt -pthread -lm)
target_link_libraries(GMAL PUBLIC ${GMAL_LINK_TARGET_FLAGS})

