find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    option(INIT_SUBMODULE "Checkout submodules during build" ON)
    if(INIT_SUBMODULE)
        message(STATUS "Initializing git submodules")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with exit code ${GIT_SUBMOD_RESULT}!")
        endif()
    endif()
endif()