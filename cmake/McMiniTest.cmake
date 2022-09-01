# McMiniTest: A CMake module for including
# GoogleTest cases and for testing custom
# programs against McMini

include(GoogleTest)

# @brief: Adds a test case for CTest based
# on the contents of a file containing Google Tests
#
# Use this macro to include tests written using
# Google Test. All tests included in all of the
# files provided will be combined into a single
# executable run by CTest
#
# @param TESTNAME: The name of the test suite
# generated for CTest. CTest will run an executable
# with this name containing all of the test files
# provided as subsequent arguments
#
# @param ARGN: The source files containing the raw
# Google Test code. All of the tests contained in
# the files listed following the name of the test
# suite will be packaged and run as a single
# executable by CTest
function(add_gtest_suite TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    target_include_directories(${TESTNAME} ${MCMINI_GOOGLE_TEST_INCLUDE_DIR})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)

    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )

    if (VERBOSE_TESTING)
        message(STATUS "GTest test suite ${TESTNAME} added")
    endif()
endfunction()

#
#
#
function(mcmini_test_against_executable TESTNAME EXECUTABLE)

endfunction()

#
#
#
function(add_executable_from_source SOURCE SUFFIX LIBRARIES)
    set(EXECUTABLE_NAME "")

    string(TOUPPER ${SUFFIX} ${SUFFIX})
    if (SUFFIX STREQUAL "C")
        string(REGEX MATCH "*.c" "EXECUTABLE_NAME" ${SOURCE})
    elseif(SUFFIX STREQUAL "CPP")
        string(REGEX MATCH "*.cpp" "EXECUTABLE_NAME" ${SOURCE})
    else()
        message(FATAL_ERROR "McMini currently only supports *.c and *.cpp exectuables")
    endif()

    add_executable("${EXECUTABLE_NAME}" "${SOURCE}" "${ARGN}")
    target_link_libraries("${EXECUTABLE_NAME} ${LIBRARIES}")
endfunction()
