// Dining philosophers solution with mutex
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NUM_THREADS (3)
#define N           (NUM_THREADS)

struct forks {
  int philosopher;
  pthread_mutex_t *left_fork;
  pthread_mutex_t *right_fork;
  pthread_mutex_t *dining_mut;
  pthread_cond_t *dining_fork;
} forkss[NUM_THREADS];

int data               = 0;
int forks[NUM_THREADS] = {0};

pthread_mutex_t mutex;
pthread_cond_t condvar;

void
eat(int i)
{
  pthread_mutex_lock(&mutex);
  if (forks[i] != 0 || forks[(i + 1) % N] != 0) {
    pthread_cond_wait(&condvar, &mutex);
  }
  forks[i] = forks[(i + 1) % N] = 1;
  pthread_mutex_unlock(&mutex);

  // Do eating
  pthread_mutex_lock(&mutex);
  int cond1 = forks[i] != 0;
  int cond2 = forks[(i + 1) % N] != 0;
  int cond  = cond1 && cond2;
  printf("%d %d\n", cond1, cond2);
  if (!cond) { printf("ASSERTION FAILED\n"); }

  // assert(forks[i] != 0 && forks[(i + 1) % N] != 0);

  // McMini cannot handle assertions
  // -> if false -> print trace? what should happen?
  forks[i] = forks[(i + 1) % N] = 0;
  pthread_cond_signal(&condvar);
  pthread_mutex_unlock(&mutex);
}

int
main(int argc, char *argv[])
{
  printf("Phils eating!\n");
  pthread_t thread[NUM_THREADS];

  memset(forks, 0, sizeof(forks));

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&condvar, NULL);

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&thread[i], NULL, &eat, (void *)i);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(thread[i], NULL);
  }

  return 0;
}
