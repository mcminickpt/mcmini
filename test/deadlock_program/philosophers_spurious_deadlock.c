// Dining philosophers solution with mutex
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int NUM_THREADS;  // number of philosophers
int DEBUG;  // debug flag

struct forks {
  int philosopher;
  pthread_mutex_t *left_fork;
  pthread_mutex_t *right_fork;
  pthread_mutex_t *dining_mut;
  pthread_cond_t *dining_fork;
} *forkss;

int data;
int* forks;

pthread_mutex_t mutex;
pthread_cond_t condvar;

void* eat(void* i_arg)
{
  int i = *((int*)i_arg);
  pthread_mutex_lock(&mutex);
  if (forks[i] != 0 || forks[(i + 1) % NUM_THREADS] != 0) {
    if(DEBUG) printf("Philosopher %d is waiting\n", i);
    pthread_cond_wait(&condvar, &mutex);
  }
  forks[i] = forks[(i + 1) % NUM_THREADS] = 1;
  pthread_mutex_unlock(&mutex);

  if (DEBUG) {
    printf("Philosopher %d just ate.\n", i);
  }

  pthread_mutex_lock(&mutex);
  int cond1 = forks[i] != 0;
  int cond2 = forks[(i + 1) % NUM_THREADS] != 0;
  int cond  = cond1 && cond2;
  if(DEBUG) printf("%d %d\n", cond1, cond2);
  if (!cond) { printf("ASSERTION FAILED\n"); }

  forks[i] = forks[(i + 1) % NUM_THREADS] = 0;
  pthread_cond_signal(&condvar);
  pthread_mutex_unlock(&mutex);

  return NULL;
}

int main(int argc, char *argv[])
{
  if(argc != 3){
    printf("Usage: %s NUM_PHILOSOPHERS DEBUG\n", argv[0]);
    return 1;
  }

  NUM_THREADS = atoi(argv[1]);
  DEBUG = atoi(argv[2]);

  if(DEBUG) printf("Phils eating!\n");
  pthread_t thread[NUM_THREADS];

  forks = malloc(NUM_THREADS * sizeof(int));
  memset(forks, 0, NUM_THREADS * sizeof(int));

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&condvar, NULL);

  forkss = malloc(NUM_THREADS * sizeof(struct forks));

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&thread[i], NULL, &eat, (void *)&i);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(thread[i], NULL);
  }

  free(forkss);
  free(forks);

  return 0;
}
