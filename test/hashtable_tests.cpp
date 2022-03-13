extern "C" {
    #include "../src/hashtable.h"
};

#include "test_common.h"
#include "gtest/gtest.h"

hash_t
sample_hash_function(void *key)
{
    return (hash_t)key;
}

bool
sample_hash_equality(void * key1, void * key2)
{
    return key1 == key2;
}

class hash_table_test : public ::testing::Test {
protected:
    hash_table_ref table;
    hash_table_iter_ref iter;

    void * const keyA = (void*)0x1;
    void * const valueA = (void*)0x2;
    void * const keyB = (void*)0x3;
    void * const valueB = (void*)0x4;

public:

    void SetUp() override {
        table = hash_table_create(sample_hash_function, sample_hash_equality);
        iter = hash_table_iter_create(table);
    }

    void TearDown() override {
        hash_table_destroy(table);
        hash_table_iter_destroy(iter);
    }
};

TEST_F(hash_table_test, test_create_hash_table_with_null_arguments) {
    ASSERT_NULL(hash_table_create(nullptr, nullptr));
    ASSERT_NULL(hash_table_create(nullptr, sample_hash_equality));
    ASSERT_NULL(hash_table_create(sample_hash_function, nullptr));
}

TEST_F(hash_table_test, test_assumptions_of_new_hash_table) {
    ASSERT_TRUE(hash_table_is_empty(table));
    ASSERT_ZERO(hash_table_count(table));
    ASSERT_FALSE(hash_table_iter_has_next(iter));
}

TEST_F(hash_table_test, test_destroy_with_null_argument) {
    EXPECT_NO_THROW(hash_table_destroy(nullptr));
}

TEST_F(hash_table_test, test_copy_with_null_argument) {
    EXPECT_NO_THROW(hash_table_copy(nullptr));
}

TEST_F(hash_table_test, test_copy_creates_equivalent_table) {

}

TEST_F(hash_table_test, test_count) {
    ASSERT_ZERO(hash_table_count(table));

    // Add values
    hash_table_set(table, keyA, valueA);
    ASSERT_EQ(hash_table_count(table), 1);

    hash_table_set(table, keyB, valueB);
    ASSERT_EQ(hash_table_count(table), 2);

    // Remove the values
    hash_table_remove(table, keyA);
    ASSERT_EQ(hash_table_count(table), 1);

    hash_table_remove(table, keyA);
    ASSERT_ZERO(hash_table_count(table));

    // Add a value whose key is equivalent twice
    hash_table_set(table, keyA, valueA);
    ASSERT_EQ(hash_table_count(table), 1);

    hash_table_set(table, keyA, valueB);
    ASSERT_EQ(hash_table_count(table), 1); // Still only one (replaced value with same key)
}

TEST_F(hash_table_test, test_hash_table_set_with_null_arguments) {
    EXPECT_NO_THROW(hash_table_set(nullptr, nullptr, nullptr));
    EXPECT_NO_THROW(hash_table_set(nullptr, keyA, nullptr));
    EXPECT_NO_THROW(hash_table_set(nullptr, nullptr, valueA));
    EXPECT_NO_THROW(hash_table_set(table, nullptr, nullptr));
    EXPECT_NO_THROW(hash_table_set(table, nullptr, valueA));
    EXPECT_NO_THROW(hash_table_set(table, keyA, nullptr));
}