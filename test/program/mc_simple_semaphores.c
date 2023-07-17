#include "mcmini/mcmini.h"
#include "mcmini/mcmini_wrappers.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t sem;
int DEBUG = 0;

int main(int argc, char *argv[])
{
  if(argc != 3){
    printf("Usage: %s START_NUM DEBUG\n", argv[0]);
    return 1;
  }

  int START_NUM = atoi(argv[1]);
  DEBUG = atoi(argv[2]);

  mcmini_main();
  mc_sem_init(&sem, 0, START_NUM);

  for (int i = 0; i < START_NUM; i++) {
    mc_sem_wait(&sem);
    if(DEBUG)
        printf("Semaphore decreased, current value: %d\n", START_NUM - i - 1);
  }

  mc_sem_post(&sem);
  if(DEBUG)
    printf("Semaphore increased, current value: 1\n");

  mc_sem_post(&sem);
  if(DEBUG)
    printf("Semaphore increased, current value: 2\n");

  mc_sem_wait(&sem);
  if(DEBUG)
    printf("Semaphore decreased, current value: 1\n");

  mc_sem_wait(&sem);
  if(DEBUG)
    printf("Semaphore decreased, current value: 0\n");

  return 0;
}
