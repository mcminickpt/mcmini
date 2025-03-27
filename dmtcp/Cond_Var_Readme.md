This document provides a detailed explanation of the algorithm used to manage condition variables 
to support checkpoint and replay and model checking at restart time.

## Thread State Tracking in Condition Variables
Each condition variable maintains a waiting_threads queue. The queue contains thread_queue_node entries that store:
- The thread's ID (runner_id_t)
- The thread's condition variable state (thread_cv_state)
## Recording Phase
Track condition variable states and thread queus to enable replay at restart time.
Each state in condition_variable_state corresponds to a phase in the lifescycle
  of a condition variable. The status of a condition variable can be one of the following:
  - CV_UNINITIALIZED: The condition variable has not been initialized yet via mc_pthread_cond_init.
  - CV_INITIALIZED: The condition variable has been initialized and ready for use (post mc_pthread_cond_init).
  - CV_TRANSITIONAL: A thread is releasing the mutex but hasn't fully entered the wait state yet
  (mc_pthread_cond_wait in progress), hence entered outer waiting room. This prevents checkpointing during the 
  unsafe gap between mutex unlock and wait.
  - CV_WAITING: The thread has successfully entered the wait state i.e, consumed the signal or successfully returned
  from libpthread_cond_timed_wait. (mc_pthread_cond_wait).
  - CV_SIGNALLED: A signal/broadcast has been sent; the condition variable resumes operation (mc_pthread_cond_signal/broadcast).
  - CV_DESTROYED: The condition variable has been destroyed (post mc_pthread_cond_destroy).
  Note that the status of a condition variable can be CV_SIGNALLED even when there are no threads waiting on it.
  This is because a thread can signal a condition variable even when no threads are waiting on it (spurious wakeup).

## Thread State Transitions

When a thread calls pthread_cond_wait():
- It's added to the waiting queue with state CV_PREWAITING. 
- After successfully entering wait, its state changes to CV_WAITING. 

When signaled:
- Its state changes to CV_SIGNALED
- The thread eventually dequeues and processes the signal

## Lost Wakeup Tracking

- The system tracks lost wakeups when signals are sent to condition variables with no waiting threads.
- These are recorded to ensure correct reconstruction during restart. 

## Serializing Thread-Condition Relationships
The system uses a two-level approach to serialize thread relationships:

### CV_WAITERS_QUEUE Entries

- For each thread in a condition variable's waiting queue, a specialized entry is created. 
- Contains: condition variable location, thread ID, and thread's CV state
- This preserves the complete thread queue state including waiting order

### State Preservation

- During serialization, the system traverses each condition variable's waiting queue. 
- For each thread, it records its specific relationship with the condition variable.
- This enables accurate reconstruction during deserialization. 

## Post-Restart Thread State Reconstruction
During restart, the system reconstructs condition variable states and thread relationships:

### Creating State Map

- CV_WAITERS_QUEUE entries are processed to build a cv_waiting_threads map
- Maps each condition variable address to an ordered list of (thread_id, cv_state) pairs

### Policy-Based Management
- Each condition variable has a policy object (ConditionVariableArbitraryPolicy) that:
    - Maintains thread waiting queue order
    - Preserves thread-specific state for each queued thread
    - Manages signaling and state transitions

### Thread State Re-establishment
- During model object creation, waiting threads are added with their correct states
- The add_waiter_with_state() method ensures each thread's state is preserved

## Post-Restart Handling By Condition Variable State

### Case 1: CV_UNINITIALIZED
- Thread behavior: No special handling required
- System action: Condition variable will be initialized when first accessed

### Case 2: CV_INITIALIZED
- Thread behavior: Thread sends COND_INIT_TYPE message via mailbox
- System action:
    - Scheduler initializes condition variable using libpthread_cond_init()
    - No thread state tracking needed as no threads are waiting

### Case 3: CV_PREWAITING
- Thread behavior:
    - Thread sends COND_ENQUEUE_TYPE message to re-enter waiting queue
    - Thread waits for scheduler acknowledgment
    - Thread sends COND_WAIT_TYPE to block until signaled
- System action:
    - Scheduler adds thread to condition variable's waiting queue with CV_PREWAITING state
    - Scheduler ensures mutex is released (critical for preventing deadlocks)
    - Maintains pre-checkpoint thread state to ensure correct handling

### Case 4: CV_WAITING
- Thread behavior:
    - Thread sends COND_ENQUEUE_TYPE message to re-register in waiting queue
    - Thread sends COND_WAIT_TYPE to re-enter wait state
- System action:
    - Scheduler adds thread to waiting queue with CV_WAITING state
    - Reconstructs exact pre-checkpoint waiting order
    - Thread properly blocks waiting for signal

### Case 5: CV_SIGNALED
- Thread behavior:
    - If thread was signaled pre-checkpoint, it resumes execution
    - If thread was waiting, it follows CV_WAITING behavior
- System action:
    - Scheduler identifies which threads were signaled pre-checkpoint
    - These threads are added to "wake groups" in the policy
    - Ensures signaled threads receive priority for resumption
    - Maintains proper FIFO ordering for signal processing

### Case 6: CV_DESTROYED
- Thread behavior: Thread sends COND_DESTROY_TYPE message
- System action:
    - Scheduler verifies no threads are waiting (or handles error)
    - Resources associated with condition variable are properly released
    - No further operations allowed on the condition variable