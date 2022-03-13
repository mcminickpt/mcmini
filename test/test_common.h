#ifndef DPOR_TESTS_TEST_COMMON_H
#define DPOR_TESTS_TEST_COMMON_H

#include "gtest/gtest.h"

#define ASSERT_NULL(expression) ASSERT_EQ(expression, nullptr)
#define ASSERT_ZERO(expression) ASSERT_EQ(expression, 0)
#define ASSERT_NEG_ONE(expression) ASSERT_EQ(expression, -1)

#endif //DPOR_TESTS_TEST_COMMON_H
