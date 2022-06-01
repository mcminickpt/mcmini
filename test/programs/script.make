#!/bin/zsh

make philosophers_semaphores
make simple_barrier_with_threads
make simple_cond_broadcast_with_semaphore

gcc -O3 -pthread philosophers_custom_semaphores.c ../CustomSemaphore.c -o philosophers_custom_semaphores

gcc -O3 -pthread simple_custom_barrier_with_threads.c ../CustomBarrier.c -o simple_custom_barrier_with_threads

gcc -O3 -pthread simple_custom_cond_broadcast_with_semaphore.c ../CustomConditionVariable.c -o simple_custom_cond_broadcast_with_semaphore


mv philosophers_semaphores ../../src/build
mv simple_barrier_with_threads ../../src/build
mv simple_cond_broadcast_with_semaphore ../../src/build
mv philosophers_custom_semaphores ../../src/build
mv simple_custom_barrier_with_threads ../../src/build
mv simple_custom_cond_broadcast_with_semaphore ../../src/build
