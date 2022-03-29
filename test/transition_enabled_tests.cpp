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
        t.thread = source_thread;
        t.operation.type = THREAD_LIFECYCLE;
        t.operation.thread_operation.type = type;
        t.operation.thread_operation.thread = target_thread;
        return t;
    }

    transition
    make_mutex_transition(thread_ref source_thread, mutex_operation_type type, mutex target_mutex)
    {
        transition t;
        t.thread = source_thread;
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

private:

    int t_next = 0;
    thread test_threads[100];
};

TEST_F(transition_enabled_test, test_thread_create) {

    thread_ref thread1 = make_thread();

    transition t1 = make_thread_transition(thread1, THREAD_START, thread1);
    ASSERT_TRUE(transition_enabled(&t1));
}
