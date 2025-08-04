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

void enqueue_thread(thread_queue *queue, runner_id_t thread, condition_variable_status cv_state) {
  thread_queue_node *node = create_thread_queue_node();
  node->thread = thread;
  node->thread_cv_state = cv_state;
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
  while(queue->front != NULL){
    if (queue->front->thread_cv_state == CV_WAITING) {
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
        queue->front = queue->front->next; // Move front pointer to the next node
    }
    // If we reach here, it means there are no threads in the queue with CV_WAITING state
  return -1;  
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
    fprintf(stdout,"%ld (%d) -> ", (long)current->thread,current->thread_cv_state);fflush(stdout);
    current = current->next;
  }
  fprintf(stdout,"NULL\n");fflush(stdout);
}

condition_variable_status get_thread_cv_state(thread_queue *queue, runner_id_t thread) {
  thread_queue_node *current = queue->front;
  while (current != NULL) {
    if (current->thread == thread) {
      return current->thread_cv_state;
    }
    current = current->next;
  }
  fprintf(stderr, "Error get_thread_cv: Thread %d not found in the queue\n", (int)thread);
  exit(EXIT_FAILURE);
}

void update_thread_cv_state(thread_queue *queue, runner_id_t thread, condition_variable_status new_state) {
  thread_queue_node *current = queue->front;
  while (current != NULL) {
    if (current->thread == thread) {
      current->thread_cv_state = new_state;
      return; // Successfully updated
    }
    current = current->next;
  }
  fprintf(stderr, "Error update_thread_cv_state: Thread %d not found in the queue\n", (int)thread);
  exit(EXIT_FAILURE);
}

runner_id_t get_waiting_thread_node(thread_queue *queue) {
  thread_queue_node *current = queue->front;
  while (current != NULL) {
    // Return the first node in the queue
    // This can be used to get the first waiting thread
    if (current->thread_cv_state == CV_WAITING) {
      return current->thread; // Found a waiting thread
    }
    current = current->next;
  }
  // If no waiting thread found, return NULL
  return RID_INVALID; // No waiting thread found
}

int remove_thread_from_queue(thread_queue* queue, runner_id_t tid) {
  if (queue == NULL || queue->front == NULL) {
    return -1; // Empty queue or invalid queue
    }
    
  // Special case: removing the first node
  if (queue->front->thread == tid) {
    thread_queue_node* to_remove = queue->front;
    queue->front = queue->front->next;
        
    // If we removed the last node, update rear pointer
    if (queue->front == NULL) {
      queue->rear = NULL;
    }
        
    free(to_remove);
    queue->size--;
    return 0;
  }
    
  // Search through the list for the node to remove
  thread_queue_node* current = queue->front;
  while (current->next != NULL) {
    if (current->next->thread == tid) {
      thread_queue_node* to_remove = current->next;
            
      // Update the next pointer to skip the node being removed
      current->next = to_remove->next;
            
      // If we're removing the last node, update rear pointer
      if (to_remove == queue->rear) {
        queue->rear = current;
      }
            
      free(to_remove);
      queue->size--;
      return 0;
    }
    current = current->next;
  }
    
  return -1; // Thread ID not found in queue
}