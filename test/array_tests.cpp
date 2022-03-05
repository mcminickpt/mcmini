extern "C" {
    #include "../src/array.h"
};

#include "test_common.h"
#include "gtest/gtest.h"

class array_test : public ::testing::Test {
public:
    array_ref test_array = nullptr;

protected:
    void SetUp() override {
        this->test_array = array_create();
    }

    void TearDown() override {
        array_destroy(this->test_array, nullptr);
    }

    // Actual addresses don't matter here
    // Sample values used in test cases
    const int *const a1 = (int*)0x1;
    const int *const a2 = (int*)0x2;
    const int *const a3 = (int*)0x3;
    const int *const a4 = (int*)0x4;
};

array_ref array_clone(void**, size_t);
void array_destroy(array_ref, void(* free_each)(void *));
void array_destroy_opaque(void*);

void *array_get_last(array_refc);
void *array_get_first(array_refc);
void array_set(array_ref, uint32_t index, const void **data);
void array_swap(array_ref, uint32_t i1, uint32_t i2);

void array_append(array_ref, const void *data);
void array_append_array(array_ref, const struct array*);
void array_insert(array_ref, uint32_t index, const void *data);
void *array_remove(array_ref, uint32_t index);
void *array_remove_first(array_ref);
void *array_remove_last(array_ref);


array_ref array_shallow_cpy(array_refc);
array_ref array_deep_cpy(array_refc, void*(*  cpy)(void * ));

TEST_F(array_test, test_array_create) {
    array_ref test = array_create();
    ASSERT_TRUE(array_is_empty(test));
    ASSERT_TRUE(array_count(test) == 0);
    ASSERT_NULL(array_get(test, 0));
    array_destroy(test, nullptr);
}

TEST_F(array_test, test_array_clear) {
    array_append(test_array, a1);
    array_append(test_array, a2);
    array_append(test_array, a3);

    array_clear(test_array);
    ASSERT_TRUE(array_is_empty(test_array));
    ASSERT_TRUE(array_count(test_array) == 0);
    ASSERT_NULL(array_get(test_array, 0));

    // Ensure the array works as expected after clearing it
    array_append(test_array, a2);
    array_append(test_array, a3);
    ASSERT_EQ(array_get(test_array, 0), a2);
    ASSERT_EQ(array_get(test_array, 1), a3);
}

TEST_F(array_test, test_array_clear_twice) {
    array_append(test_array, a1);
    array_append(test_array, a2);
    array_append(test_array, a3);

    array_clear(test_array);
    array_clear(test_array);
    ASSERT_TRUE(array_is_empty(test_array));
    ASSERT_TRUE(array_count(test_array) == 0);
    ASSERT_NULL(array_get(test_array, 0));

    // Ensure the array works as expected after clearing it
    array_append(test_array, a2);
    array_append(test_array, a3);
    ASSERT_EQ(array_get(test_array, 0), a2);
    ASSERT_EQ(array_get(test_array, 1), a3);
}

TEST_F(array_test, test_array_clear_null) {
    array_clear(nullptr); // Ensure that nothing happens
}

TEST_F(array_test, test_array_count_and_get) {
    ASSERT_EQ(array_count(nullptr), 0);
    ASSERT_EQ(array_count(this->test_array), 0);

    array_append(this->test_array, a1);
    array_append(this->test_array, a2);
    ASSERT_EQ(array_count(this->test_array), 2);
    ASSERT_EQ(array_get(this->test_array, 0), a1);
    ASSERT_EQ(array_get(this->test_array, 1), a2);

    int *b2 = (int*)array_remove_last(this->test_array);
    ASSERT_EQ(a2, b2);
    ASSERT_EQ(array_count(this->test_array), 1);
    ASSERT_EQ(array_get(this->test_array, 0), a1);

    int *b1 = (int*)array_remove_last(this->test_array);
    ASSERT_EQ(a1, b1);
    ASSERT_EQ(array_count(this->test_array), 0);
}

TEST_F(array_test, test_array_is_empty) {
    ASSERT_TRUE(array_is_empty(nullptr));
    ASSERT_TRUE(array_is_empty(test_array));
    ASSERT_EQ(array_count(test_array), 0);
}

TEST_F(array_test, test_array_get_element) {

    ASSERT_NULL(array_get(nullptr, 0));
    ASSERT_NULL(array_get(this->test_array, 0));

    array_append(test_array, a1);
    array_append(test_array, a2);
    array_append(test_array, a3);
    array_append(test_array, a4);

    ASSERT_EQ(array_get(test_array, 0), a1);
    ASSERT_EQ(array_get(test_array, 1), a2);
    ASSERT_EQ(array_get(test_array, 2), a3);
    ASSERT_EQ(array_get(test_array, 3), a4);
}

TEST_F(array_test, test_array_get_last) {

    ASSERT_NULL(array_get_last(nullptr));
    ASSERT_NULL(array_get_last(test_array));

    array_append(test_array, a1);
    array_append(test_array, a2);
    array_append(test_array, a3);
    array_append(test_array, a4);

    ASSERT_EQ(array_get_last(test_array), a4);
    array_remove_last(test_array);
    ASSERT_EQ(array_get_last(test_array), a3);
    array_remove_last(test_array);
    ASSERT_EQ(array_get_last(test_array), a2);
    array_remove_last(test_array);
    ASSERT_EQ(array_get_last(test_array), a1);
    array_remove_last(test_array);
    ASSERT_NULL(array_get_last(test_array));
}

TEST_F(array_test, test_array_remove_last) {

    ASSERT_NULL(array_remove_last(nullptr));
    ASSERT_TRUE(array_is_empty(test_array));
    ASSERT_NULL(array_remove_last(test_array));

    array_append(test_array, a1);
    array_append(test_array, a2);
    array_append(test_array, a3);
    array_append(test_array, a4);

    ASSERT_EQ(array_remove_last(test_array), a4);
    ASSERT_EQ(array_remove_last(test_array), a3);
    ASSERT_EQ(array_remove_last(test_array), a2);
    ASSERT_EQ(array_remove_last(test_array), a1);
    ASSERT_NULL(array_remove_last(test_array));
}