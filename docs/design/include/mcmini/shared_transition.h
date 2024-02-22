#ifndef INCLUDE_SHARED_TRANSITION_H
#define INCLUDE_SHARED_TRANSITION_H
#include <stdint.h>

typedef uint64_t tid_t;

// Define an enumeration for type info
// Enum for TransitionTypeInfo
typedef enum {
  MutexInit,
  MutexLock,
  MutexUnlock,
  SemaphoreInit,
  SemaphoreWait,
  SemaphorePost,
  ThreadCreate,
  ThreadJoin,
} TransitionTypeInfo;

struct TypeInfo {
  TransitionTypeInfo type;
};

struct SharedTransition {
  tid_t executor;
  struct TypeInfo type;
};

#endif


/*
#ifndef INCLUDE_SHARED_TRANSITION_H
#define INCLUDE_SHARED_TRANSITION_H

#include <stdint.h>

typedef uint64_t tid_t;

// Define an enumeration for type info
enum TypeInfo {
    TypeInfo_Int,
    TypeInfo_Char,
    // Add other type information as needed
};

// Define a structure equivalent to SharedTransition
struct SharedTransition {
    tid_t executor;
    enum TypeInfo type;  // Use enum for type information
};

// Function to create a SharedTransition
struct SharedTransition createSharedTransition(tid_t executor, enum TypeInfo type) {
    struct SharedTransition transition;
    transition.executor = executor;
    transition.type = type;
    return transition;
}

#endif

*/