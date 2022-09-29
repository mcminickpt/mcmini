#include "rwwlock.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_READERS       1
#define NUM_TYPE1_WRITERS 1
#define NUM_TYPE2_WRITERS 1
#define NUM_LOOP          1

#define printf

pthread_rwwlock_t rw;

void *
reader(void *notused)
{
  for (int i = 0; i < NUM_LOOP; i++) {
    // acquire resource
    pthread_rwwlock_rdlock(&rw);

    // use resource (we fake this by sleeping)
    printf("reader is reading\n");
    sleep(1);
    // release resource
    pthread_rwwlock_unlock(&rw);
  }
  return NULL;
}

void *
writer1(void *notused)
{
  for (int i = 0; i < NUM_LOOP; i++) {
    // acquire resource
    pthread_rwwlock_wr1lock(&rw);

    // use resource (we fake this by sleeping)
    printf("writer 1 is writing\n");
    sleep(5);
    // release resource
    pthread_rwwlock_unlock(&rw);
  }
  return NULL;
}

void *
writer2(void *notused)
{
  for (int i = 0; i < NUM_LOOP; i++) {
    // acquire resource
    pthread_rwwlock_wr2lock(&rw);

    // use resource (we fake this by sleeping)
    printf("writer 2 is writing\n");
    sleep(5);
    // release resource
    pthread_rwwlock_unlock(&rw);
  }
  return NULL;
}

int
main()
{
  pthread_t read_thread[NUM_READERS];
  pthread_t writer1_thread[NUM_TYPE1_WRITERS];
  pthread_t writer2_thread[NUM_TYPE2_WRITERS];
  pthread_rwwlock_init(&rw);

  int i;
  for (i = 0; i < NUM_READERS; i++) {
    pthread_create(&read_thread[i], NULL, reader, NULL);
  }
  for (i = 0; i < NUM_TYPE1_WRITERS; i++) {
    pthread_create(&writer1_thread[i], NULL, writer1, NULL);
  }
  for (i = 0; i < NUM_TYPE2_WRITERS; i++) {
    pthread_create(&writer2_thread[i], NULL, writer2, NULL);
  }

  for (i = 0; i < NUM_READERS; i++) {
    pthread_join(read_thread[i], NULL);
  }
  for (i = 0; i < NUM_TYPE1_WRITERS; i++) {
    pthread_join(writer1_thread[i], NULL);
  }
  for (i = 0; i < NUM_TYPE2_WRITERS; i++) {
    pthread_join(writer2_thread[i], NULL);
  }
  return 0;
}
