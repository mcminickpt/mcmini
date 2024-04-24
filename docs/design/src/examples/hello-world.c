#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    pthread_mutex_t mut;
    pthread_mutex_init(&mut, NULL);
    // pthread_mutex_lock(&mut);
    // printf("Hello world!\n");
    // pthread_mutex_unlock(&mut);
    // pthread_mutex_destroy(&mut);
    return 0;
}
