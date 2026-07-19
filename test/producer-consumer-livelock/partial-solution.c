// Remember to compile as:  gcc -c -pthread THIS_FILE
// This generates a .o file that can be linked to your test program,
//   myhw.c, which will contain the 'main()' routine.

// The motivation for semaphores is the producer-consumer problem.
// See Chapter 31 of ostep.org -
//   especially page 10: Figure 31.12: Adding Mutual Exclusion (Correctly)
// Note that in class, I preferred to write:
//   pthread_mutex_lock(&mutex); ...  pthread_mutex_unlock(&mutex);
// while the textbook prefers the following pattern that does the same thing:
//   sem_t mutex; sem_wait(&mutex); ...  sem_post(&mutex);

#include <unistd.h>
#include <pthread.h>

// Recall that the count for a semaphore has the following interpretation:
//   If count >= 0, then it's the number of calls to sem_wait() to be allowed.
//   If count < 0, then it's the number of threads waiting on this semaphore.
// I refer to this as a "consistency criterion".   The count always tells
//   you the number of threads that are blocked (that are waiting).
// So, sem_wait() essentially does:  {count--; if (count < 0) block();}
struct sem {
  int count;
  pthread_mutex_t mutex;
};
typedef struct sem sem_t;
// Unlike Java, C/C++ require a semicolon to end the 'struct sem' statement.

int sem_init(sem_t *sem, int ignore, int init) {
  sem->count = init;
  // Anybody who wants to use sem->count should acquire this lock first.
  pthread_mutex_init(&(sem->mutex), NULL);  // See 'man pthread_mutex_init'
}

int sem_post(sem_t *sem) {
  pthread_mutex_lock( &(sem->mutex) );
  sem->count += 1;
  pthread_mutex_unlock( &(sem->mutex) );
}

void block(sem_t *sem); // Used by sem_wait()

int sem_wait(sem_t *sem) {
  pthread_mutex_lock( &(sem->mutex) );
  sem->count -= 1;
  if (sem->count < 0) {
    // We must release lock, or no one can ever call sem_post() for us.
    pthread_mutex_unlock( &(sem->mutex) );
    block(sem);
    pthread_mutex_lock( &(sem->mutex) );
  }
  pthread_mutex_unlock( &(sem->mutex) );
}

void block(sem_t *sem) {
  int mycount;
  while (1) {
    pthread_mutex_lock( &(sem->mutex) );
    mycount = sem->count;
    if (mycount >= 0) {
      pthread_mutex_unlock( &(sem->mutex) );
      return;
    }
    pthread_mutex_unlock( &(sem->mutex) );
    sleep(1); // sleep for a second, and check later if count >= 0.
  }
}

// This is a partial solution.
// The above code works correctly when there are two threads,
//   but when there are three or more threads, a bug will appear.
//   The bug appears when more than one thread is blocked:
//   e.g., two threads blocked, one thread ready to post.
//   In this case, our consistency criterion tells us that 'count == -2'.
// Now, suppose some additional thread calls 'sem_post(&mysem)',
//   causing the 'count' to change from -2 to -1.
// Ideally, we would want one of the two threads that is trapped
//   inside 'block()' to return.
// Unfortunately, 'mycount' will now have the value '-1', and both
//   of the two threads will conclude that they cannot return from 'blcck()'.
// So, the consistency criterion is violated:  two threads blocked, count == -1
// The essence of the homework is _either_ to diagnose this bug, or else
//   to fix this bug (depending on the homework spec).
//   If the homework is to fix the bug, then you must make it work
//   for correctly for arbitrarily many threads.
