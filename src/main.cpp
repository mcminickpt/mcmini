#include "mcmini/export/rwwlock.h"
#include "mcmini/mcmini.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_READERS 2
#define NUM_WRITERS 1
#define NUM_LOOP    1

pthread_rwwlock_t rw;

void *
reader(void *notused)
{
  for (int i = 0; i < NUM_LOOP; i++) {
    // acquire resource
    mc_pthread_rwwlock_rdlock(&rw);

    // use resource (we fake this by sleeping)
    // printf("reader is reading\n");
    // sleep(1);
    // release resource
    mc_pthread_rwwlock_unlock(&rw);
  }
  return NULL;
}

void *
writer(void *notused)
{
  for (int i = 0; i < NUM_LOOP; i++) {
    // acquire resource
    mc_pthread_rwwlock_wr1lock(&rw);

    // use resource (we fake this by sleeping)
    // printf("writer is writing\n");
    // sleep(5);
    // release resource
    mc_pthread_rwwlock_unlock(&rw);
  }
  return NULL;
}

int
main()
{
  mcmini_main();
  pthread_t read_thread[NUM_READERS];
  pthread_t write_thread[NUM_WRITERS];
  mc_pthread_rwwlock_init(&rw);

  int i;
  for (i = 0; i < NUM_READERS; i++) {
    mc_pthread_create(&read_thread[i], NULL, reader, NULL);
  }
  for (i = 0; i < NUM_WRITERS; i++) {
    mc_pthread_create(&write_thread[i], NULL, writer, NULL);
  }

  for (i = 0; i < NUM_READERS; i++) {
    mc_pthread_join(read_thread[i], NULL);
  }
  for (i = 0; i < NUM_WRITERS; i++) {
    mc_pthread_join(write_thread[i], NULL);
  }
  return 0;
}
