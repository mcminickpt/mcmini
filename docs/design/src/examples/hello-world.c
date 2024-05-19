#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

pthread_mutex_t mut;

void *test(void* unused) {
    pthread_mutex_lock(&mut);
}

int main(int argc, char* argv[])
{
    pthread_t child;
    pthread_mutex_init(&mut, NULL);
    pthread_create(&child, NULL, &test, NULL);
    pthread_mutex_lock(&mut);
    pthread_mutex_unlock(&mut);
    return 0;
}
