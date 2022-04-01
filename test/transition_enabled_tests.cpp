extern "C" {
#include "../src/transition.h"
};

#include "test_common.h"
#include "gtest/gtest.h"

class transition_enabled_test : public ::testing::Test {
protected:

    transition
    make_thread_transition(thread_ref source_thread, thread_operation_type type, thread_ref target_thread)
    {
        transition t;
        t.thread = *source_thread;
        t.operation.type = THREAD_LIFECYCLE;
        t.operation.thread_operation.type = type;
        t.operation.thread_operation.thread = target_thread;
        return t;
    }

    transition
    make_mutex_transition(thread_ref source_thread, mutex_operation_type type, mutex target_mutex)
    {
        transition t;
        t.thread = *source_thread;
        t.operation.type = MUTEX;
        t.operation.mutex_operation.type = type;
        t.operation.mutex_operation.mutex = target_mutex;
        return t;
    }

    thread_ref
    make_thread()
    {
        return &test_threads[t_next++];
    }

    void
    TearDown() override
    {
        t_next = 0;
    }

    void
    SetUp() override
    {
        t_next = 0;
    }

private:
    int t_next = 0;
    thread test_threads[100];
};

#define TEST_THREAD_OPERATION_ENABLED(op) \
    TEST_F(transition_enabled_test, test_##op) { \
        thread_ref thread1 = make_thread(); \
        transition t1 = make_thread_transition(thread1, op, thread1); \
        ASSERT_TRUE(transition_enabled(&t1)); \
}

#define TEST_THREAD_OPERATION_DISABLED(op) \
    TEST_F(transition_enabled_test, test##op) { \
        thread_ref thread1 = make_thread(); \
        transition t1 = make_thread_transition(thread1, op, thread1); \
        ASSERT_FALSE(transition_enabled(&t1)); \
}

// Assumes that the threads are alive
TEST_THREAD_OPERATION_ENABLED(THREAD_START)
TEST_THREAD_OPERATION_ENABLED(THREAD_CREATE)
TEST_THREAD_OPERATION_ENABLED(THREAD_FINISH)
TEST_THREAD_OPERATION_DISABLED(THREAD_TERMINATE_PROCESS)

TEST_F(transition_enabled_test, test_thread_join_enabled_with_dead_thread) {
    thread_ref thread1 = make_thread();
    thread1->state = THREAD_DEAD;

    thread_ref thread_waiting = make_thread();
    thread_waiting->state = THREAD_ALIVE;

    transition t1 = make_thread_transition(thread_waiting, THREAD_JOIN, thread1);
    ASSERT_TRUE(transition_enabled(&t1));
}

TEST_F(transition_enabled_test, test_thread_join_disabled_with_alive_thread) {
    thread_ref thread1 = make_thread();
    thread1->state = THREAD_ALIVE;

    thread_ref thread_waiting = make_thread();
    thread_waiting->state = THREAD_ALIVE;

    transition t1 = make_thread_transition(thread_waiting, THREAD_JOIN, thread1);
    ASSERT_FALSE(transition_enabled(&t1));
}

TEST_F(transition_enabled_test, test_thread_join_disabled_with_sleeping_thread) {
    thread_ref thread1 = make_thread();
    thread1->state = THREAD_ALIVE;

    thread_ref thread_waiting = make_thread();
    thread_waiting->state = THREAD_SLEEPING;

    transition t1 = make_thread_transition(thread_waiting, THREAD_JOIN, thread1);
    ASSERT_FALSE(transition_enabled(&t1));
}