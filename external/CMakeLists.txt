option(INSTALL_GMOCK "Install GMock?" OFF)
option(INSTALL_GTEST "Install GTest?" OFF)

if (BUILD_TESTS)
  add_subdirectory(googletest)
  mark_as_advanced(BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
  )
endif()