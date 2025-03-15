#include "mcmini/Thread_queue.h"

#include <stdlib.h>
#include <stdio.h>

thread_queue* create_thread_queue() {
    thread_queue *queue = malloc(sizeof(thread_queue));
    if (!queue) {
        perror("Failed to allocate memory for thread queue");
        exit(EXIT_FAILURE);
    }
    queue->front = queue->rear = NULL;
    
    return queue;
}

thread_queue_node* create_thread_queue_node() {
    thread_queue_node *node = malloc(sizeof(thread_queue_node));
    if (!node) {
        perror("Failed to allocate memory for thread queue node");
        exit(EXIT_FAILURE);
    }
    node->next = NULL;
    return node;
}

void enqueue_thread(thread_queue *queue, runner_id_t thread) {
    thread_queue_node *node = create_thread_queue_node();
    node->thread = thread;

    if (queue->rear == NULL) {
        queue->front = queue->rear = node;
    } else {
        queue->rear->next = node;
        queue->rear = node;
    }
    queue->size++;
}

runner_id_t dequeue_thread(thread_queue *queue) {
    if (queue->front == NULL) {
        fprintf(stderr, "Error: Attempt to dequeue from an empty queue\n");
        exit(EXIT_FAILURE);
    }

    thread_queue_node *temp = queue->front;
    pthread_t thread = temp->thread;

    queue->front = queue->front->next;
    if (!queue->front) {
        queue->rear = NULL;
    }

    free(temp);
    queue->size--;
    return thread;
}

runner_id_t peek_thread(thread_queue *queue) {
    if (queue->front == NULL) {
        fprintf(stderr, "Error: Attempt to peek at an empty queue\n");
        exit(EXIT_FAILURE);
    }
    return queue->front->thread;
}

bool is_in_thread_queue(thread_queue *queue, runner_id_t thread) {
    thread_queue_node *current = queue->front;
    while (current != NULL) {
        if (current->thread == thread) {
            return true;
        }
        current = current->next;
    }
    return false;
}

bool is_queue_empty(thread_queue *queue) {
    return queue->front == NULL;
}

 // Added for debugging purpose:
 void print_thread_queue(const thread_queue *queue) {
    if (queue->front == NULL) {
        fprintf(stdout,"Thread queue is empty.\n");fflush(stdout);
        return;
    }
    
    fprintf(stdout,"Thread queue (size: %d): ", queue->size);fflush(stdout);
    thread_queue_node *current = queue->front;
    while (current) {
        fprintf(stdout,"%ld -> ", (long)current->thread);fflush(stdout);
        current = current->next;
    }
    fprintf(stdout,"NULL\n");fflush(stdout);
}
