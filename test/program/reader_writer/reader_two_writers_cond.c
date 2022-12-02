#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_READERS       1
#define NUM_TYPE1_WRITERS 1
#define NUM_TYPE2_WRITERS 1
#define NUM_LOOP          2

#define printf

pthread_mutex_t mutex;
pthread_cond_t cond;

int num_readers  = 0;
int num_writer1s = 0;
int num_writer2s = 0;

int num_readers_waiting  = 0;
int num_writer1s_waiting = 0;
int num_writer2s_waiting = 0;

int
read_condition()
{
  // No _active_ writers
  return (num_writer1s - num_writer1s_waiting) == 0 &&
         (num_writer2s - num_writer2s_waiting) == 0;
}

int
write1_condition()
{
  // No _active_ readers or writers of the other type
  return (num_readers - num_readers_waiting) == 0 &&
         (num_writer2s - num_writer2s_waiting) == 0 &&
         (num_writer1s - num_writer1s_waiting) ==
           1; // This thread is the 1 writer
}

int
write2_condition()
{
  return (num_readers - num_readers_waiting) == 0 &&
         (num_writer1s - num_writer1s_waiting) == 0 &&
         (num_writer2s - num_writer2s_waiting) ==
           1; // This thread is the 1 writer
}

void *
reader(void *unused)
{
  for (int i = 0; i < NUM_LOOP; i++) {
    // acquire resource
    pthread_mutex_lock(&mutex);
    num_readers++;
    while (!read_condition()) {
      num_readers_waiting++;
      pthread_cond_wait(&cond, &mutex); // wait on cond
      num_readers_waiting--;
    }
    pthread_mutex_unlock(&mutex);
    // use resource (we fake this by sleeping)
    printf("reader is reading\n");
    // sleep(1);
    // release resource
    pthread_mutex_lock(&mutex);
    num_readers--;
    pthread_cond_broadcast(
      &cond); // wake up everyone and let them try again
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void *
writer1(void *unused)
{
  for (int i = 0; i < NUM_LOOP; i++) {
    // acquire resource
    pthread_mutex_lock(&mutex);
    num_writer1s++;
    while (!write1_condition()) {
      num_writer1s_waiting++;
      pthread_cond_wait(&cond, &mutex); // wait on cond
      num_writer1s_waiting--;
    }
    pthread_mutex_unlock(&mutex);
    // use resource (we fake this by sleeping)
    printf("writer1 is writing\n");
    // sleep(5);
    // release resource
    pthread_mutex_lock(&mutex);
    num_writer1s--;
    pthread_cond_broadcast(
      &cond); // wake up everyone and let them try again
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void *
writer2(void *unused)
{
  for (int i = 0; i < NUM_LOOP; i++) {
    // acquire resource
    pthread_mutex_lock(&mutex);
    num_writer2s++;
    while (!write2_condition()) {
      num_writer2s_waiting++;
      pthread_cond_wait(&cond, &mutex); // wait on cond
      num_writer2s_waiting--;
    }
    pthread_mutex_unlock(&mutex);
    // use resource (we fake this by sleeping)
    printf("writer2 is writing\n");
    // sleep(5);
    // release resource
    pthread_mutex_lock(&mutex);
    num_writer2s--;
    pthread_cond_broadcast(
      &cond); // wake up everyone and let them try again
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

int
main()
{
  pthread_t read_thread[NUM_READERS];
  pthread_t write1_thread[NUM_TYPE1_WRITERS];
  pthread_t write2_thread[NUM_TYPE2_WRITERS];
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);

  int i;

  for (i = 0; i < NUM_READERS; i++) {
    pthread_create(&read_thread[i], NULL, reader, NULL);
  }
  for (i = 0; i < NUM_TYPE1_WRITERS; i++) {
    pthread_create(&write1_thread[i], NULL, writer1, NULL);
  }
  for (i = 0; i < NUM_TYPE2_WRITERS; i++) {
    pthread_create(&write2_thread[i], NULL, writer2, NULL);
  }

  for (i = 0; i < NUM_READERS; i++) {
    pthread_join(read_thread[i], NULL);
  }
  for (i = 0; i < NUM_TYPE1_WRITERS; i++) {
    pthread_join(write1_thread[i], NULL);
  }
  for (i = 0; i < NUM_TYPE2_WRITERS; i++) {
    pthread_join(write2_thread[i], NULL);
  }

  return 0;
}
