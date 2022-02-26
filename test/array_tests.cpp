extern "C" {
    #include "../src/array.h"
};

#include "gtest/gtest.h"

class array_test : public ::testing::Test {
public:
    array_ref test_array;

protected:
    void SetUp() override {
        this->test_array = array_create();
    }

    void TearDown() override {
        array_destroy(this->test_array, nullptr);
    }
};

TEST_F(array_test, test_initial_array_empty) {
    ASSERT_TRUE(array_is_empty(this->test_array));
    ASSERT_EQ(array_count(this->test_array), 0);
};

TEST_F(array_test, test_initial_array_empty) {
    ASSERT_TRUE(array_is_empty(this->test_array));
    ASSERT_EQ(array_count(this->test_array), 0);
};