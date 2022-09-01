#include "mcmini/MCMINI.h"
#include "mcmini/MCMINIWrappers.h"
#include <pthread.h>
#include <semaphore.h>
#define START_NUM 5

sem_t sem;

int
main(int argc, char *argv[])
{
  mc_init();
  mc_sem_init(&sem, 0, START_NUM);

  for (int i = 0; i < START_NUM; i++) { mc_sem_wait(&sem); }
  mc_sem_post(&sem);
  mc_sem_post(&sem);
  mc_sem_wait(&sem);
  mc_sem_wait(&sem);

  return 0;
}
