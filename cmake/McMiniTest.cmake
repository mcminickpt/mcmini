# McMiniTest: A CMake module for including
# GoogleTest cases and for testing custom
# programs against McMini

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
    target_include_directories(${TESTNAME} ${MCMINI_GOOGLE_TEST_INCLUDE_DIR})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
    
    if (VERBOSE_TESTING) 
        message(STATUS "GTest test suite ${TESTNAME} added")
    endif()
endfunction()

mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

function(mcmini_add_executable_test TESTNAME EXECUTABLE)

endfunction()
