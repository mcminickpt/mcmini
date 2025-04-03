This document provides a detailed explanation of the algorithm used to manage condition variables 
to support checkpoint and replay and model checking at restart time.

## Recording Phase
Track condition variable states and thread queus to enable replay at restart time.
Each state in condition_variable_state corresponds to a phase in the lifescycle
  of a condition variable. The status of a condition variable can be one of the following:
  - CV_UNINITIALIZED: The condition variable has not been initialized yet via mc_pthread_cond_init.
  - CV_INITIALIZED: The condition variable has been initialized and ready for use (post mc_pthread_cond_init).
  - CV_PREWAITING: A thread is releasing the mutex but hasn't fully entered the wait state yet
  (mc_pthread_cond_wait in progress), hence entered outer waiting room. This prevents checkpointing during the 
  unsafe gap between mutex unlock and wait.
  - CV_WAITING: The thread has successfully entered the wait state i.e, consumed the signal or successfully returned
  from libpthread_cond_timed_wait. (mc_pthread_cond_wait).
  - CV_SIGNALED: A signal/broadcast has been sent; the condition variable resumes operation (mc_pthread_cond_signal/broadcast).
  - CV_DESTROYED: The condition variable has been destroyed (post mc_pthread_cond_destroy).
  Note that the status of a condition variable can be CV_SIGNALED even when there are no threads waiting on it.
  This is because a thread can signal a condition variable even when no threads are waiting on it (spurious wakeup).

### Workflow
1. Acquire rec_list_lock:
    Ensure atomic access to the global state-tracking list (rec_list).

2. Check/Create Condition Record:
    If the condition variable is uninitialized, create a visible_object entry in rec_list with:
        type = CONDITION_VARIABLE
        cond_state.status = CV_INITIALIZED
        waiting_threads queue (initially empty).

3. Transition to CV_PREWAITING:
    Add the current thread to the waiting_threads queue.
    Set cond_state.status = CV_PREWAITING to block checkpoints during the unsafe window between mutex unlock and wait.

4. Timed Wait:
    Call libpthread_cond_timedwait with a short timeout (e.g., 2 seconds).
        On success (thread enters wait state):
        Update cond_state.status = CV_WAITING.
        On timeout:
        Retry or handle checkpoint/restart requests.

5. Signal Handling:
    When mc_pthread_cond_signal is called, dequeue the first thread from waiting_threads and mark the condition as CV_SIGNALED.

## Serializing and deserializing recoded info to create model
To accurately reconstruct the waiting threads associated with a condition variable during post-restart deserialization, the system introduces a specialized visible_object type, CV_WAITERS_QUEUE. This object is serialized during recording to capture the relationship between a condition variable and the threads waiting on it.
- The CV_WAITERS_QUEUE entries are distinct from the primary CONDITION_VARIABLE entries. While the latter stores the condition variable’s core state (e.g., CV_WAITING, interacting_thread), the former explicitly links waiting threads to their parent condition variable.
- As the FIFO is read, each CV_WAITERS_QUEUE entry is processed to populate a cv_waiting_threads map. This map associates a condition variable’s memory address (cv_location) with a vector of runner_id_t values representing threads in its wait queue.
- When translating the main CONDITION_VARIABLE entry into a model object, the cv_waiting_threads map is passed to translate_recorded_object_to_model to retrieve the list of waiting threads.
- The resulting condition_variable model object is initialized with the reconstructed waiting_thread_ids, ensuring the wait queue is restored to its pre-checkpoint state.

## Post- Restart Handling
After a checkpoint/restart, the system reconstructs the state of condition variables (pthread_cond_t) based on their pre-checkpoint state.
All the states are possible at the time of restart.

### Case 1: CV_INITIALIZED
The condition variable was initialized but not in use (no waiting threads).
- The thread sends a COND_INIT_TYPE message to the scheduler via the mailbox.
- The scheduler re-initializes the condition variable using libpthread_cond_init.
- After initialization, threads can safely use the condition variable for future waits/signals.

### Case 2: CV_PREWAITING
A thread had released the mutex but not yet entered the wait state.
- The thread sends a COND_ENQUEUE_TYPE message to the scheduler.
- The scheduler enqueues the thread into the condition’s waiting_threads queue and releases the mutex.
- The thread sends a COND_WAIT_TYPE message to block until signaled.
- The scheduler re-acquires the mutex and resumes the thread.

### Case 3: CV_WAITING
A thread was actively waiting on the condition variable (inside libpthread_cond_timedwait).
- The thread sends COND_ENQUEUE_TYPE and COND_WAIT_TYPE messages to re-enter the wait state.
- The thread blocks until the condition is signaled again.

same for CV_SIGNALED.

