// NOTE:  You can choose WRITER_PREFERRED, READER_PREFERRED, or default
// NOTE:  The bug still exists if you change the reader function to do 1 task:
//          for (i = 0; i < 1; i++) { ... }
//        However, with 2 reader tasks, running will show the deadlock.
// NOTE:  With 1 reader task, running will not show the deadlock.
//          and McMini has a bug, and fails to catch the reader-writer bug.
//        So, I recommend running as is (without McMini) in this case.

// TO COMPILE:  gcc -g3 -O0 THIS_FILE -pthread
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


// Uncomment preferred policy, or accept default (balanced) policy
//#define WRITER_PREFERRED
#define READER_PREFERRED

int num_active_readers = 0;
int num_active_writers = 0;
int num_waiting_readers = 0;
int num_waiting_writers = 0;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond;
int quiet = 0;

void *reader(void *arg) {
  int *reader_num = arg;
  int i;
  for (i = 0; i < 3; i++) {
    // Acquire permission    
    sleep(6);
    pthread_mutex_lock(&mut);
#ifdef WRITER_PREFERRED
    while (num_active_writers > 0 || num_waiting_writers > 0) /*Prefer Writer*/
#elif defined(READER_PREFERRED)
    while (num_active_writers > 0) /* Reader Preferred */
#else
    while (num_active_writers > 0) /* Default */
#endif
    {
      num_waiting_readers++;
      pthread_cond_wait(&cond, &mut);
      num_waiting_readers--;
    }
    num_active_readers++;
    pthread_mutex_unlock(&mut);
    // Do task
    if (! quiet) {
      printf("reader %d did task %d.\n", *reader_num, i+1);
    }
    // Release permission   
    pthread_mutex_lock(&mut);
    num_active_readers--;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mut);
  }
  return NULL;
}

void *writer(void *arg) {
  int *writer_num = arg;
  int i;
  for (i = 0; i < 2; i++) {
    // Acquire permission  
    sleep(6);
    pthread_mutex_lock(&mut);
    
#ifdef WRITER_PREFERRED
    while (num_active_readers > 0 || num_active_writers > 0) /* Prefer Writer */
#elif defined(READER_PREFERRED)
    while (num_active_readers > 0 || num_active_writers > 0 ||
           num_waiting_readers > 0) /* Reader Preferred */
#else
    while (num_active_readers > 0 || num_active_writers > 0) /* Default */
#endif
    {
      num_waiting_writers++;
      pthread_cond_wait(&cond, &mut);
      num_waiting_writers--;
    }
    num_active_writers++;
    pthread_mutex_unlock(&mut);
    
    // Do task
    if (! quiet) {
      printf("writer %d did task %d.\n", *writer_num, i+1);
    }
    // Release permission    
    pthread_mutex_lock(&mut);
    num_active_writers--;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mut);
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  int num_readers = 2;
  int num_writers = 2;
  int i;
  int thread_number[3] = {1, 2, 3};
  if (argc == 2 && strcmp(argv[1], "--quiet") == 0) {
    quiet = 1;
  }
  // This next stmt not needed, but McMini doesn't yet detect const. initializer
  pthread_mutex_init(&mut, NULL);
  pthread_t reader_thread[3]; // Create 3 pthread_t even if we don't use it all
  pthread_t writer_thread[3]; // Create 3 pthread_t even if we don't use it all
  pthread_cond_init(&cond, NULL); 
  for (i = 0; i < num_readers; i++) {
    // Pass in argument 'i'
    pthread_create(&reader_thread[i], NULL, reader, &thread_number[i]);
  }
  for (i = 0; i < num_writers; i++) {
    // Pass in argument 'i'
    pthread_create(&writer_thread[i], NULL, writer, &thread_number[i]);
  }

  for (i = 0; i < num_readers; i++) {
    pthread_join(reader_thread[i], NULL);
  }
  for (i = 0; i < num_writers; i++) {
    pthread_join(writer_thread[i], NULL);
  }
  return 0;
}
