#include "thread.h"

struct thread_array {
    struct thread *contents; // malloc
    const unsigned int count;
};
