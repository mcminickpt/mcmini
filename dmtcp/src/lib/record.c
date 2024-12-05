#include "mcmini/spy/checkpointing/record.h"
#include "mcmini/spy/checkpointing/rec_list.h"
#include "mcmini/spy/checkpointing/objects.h"
#include "mcmini/spy/checkpointing/transitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t dmtcp_restart_sem;
pthread_mutex_t rec_list_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pending_op_lock = PTHREAD_MUTEX_INITIALIZER;
volatile atomic_int libmcmini_mode = PRE_DMTCP_INIT;
visible_object empty_visible_obj = {.type = UNKNOWN, .location = NULL};
rec_list *head_record_mode = NULL;
rec_list *current_record_mode = NULL;

transition invisible_operation_for_this_thread(void) {
  transition t = {.type = INVISIBLE_OPERATION_TYPE, .executor = pthread_self()};
  return t;
}

rec_list *find_object(void *addr, rec_list *head) {
  for (rec_list *node = head; node != NULL; node = node->next) {
    if (node->vo.location == addr) return node;
  }
  return NULL;
}

rec_list *find_thread_record_mode(pthread_t thrd) {
  for (rec_list *node = head_record_mode; node != NULL; node = node->next) {
    if (node->vo.type == THREAD &&
        pthread_equal(thrd, node->vo.thrd_state.pthread_desc))
      return node;
  }
  return NULL;
}

rec_list *find_object_record_mode(void *addr) {
  return find_object(addr, head_record_mode);
}

rec_list *add_rec_entry(const visible_object *vo, rec_list **head, rec_list **current) {
  rec_list *new_node = (rec_list *)malloc(sizeof(rec_list));
  if (new_node == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  new_node->vo = *vo;
  if (*head == NULL) {
    *head = new_node;
    *current = new_node;
  }
  else {
    (*current)->next = new_node;
    *current = new_node;
  }
  return new_node;
}

rec_list *add_rec_entry_record_mode(const visible_object *vo) {
  rec_list *new_node = (rec_list *)malloc(sizeof(rec_list));
  if (new_node == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  new_node->vo = *vo;
  new_node->next = NULL;
  if (head_record_mode == NULL) {
    head_record_mode = new_node;
    current_record_mode = new_node;
  }
  else {
    current_record_mode->next = new_node;
    current_record_mode = new_node;
  }
  return new_node;
}

//debugging puropses, will remove later
// void print_rec_list(const rec_list *head) {
//   const rec_list *current = head;
//   while (current != NULL) {
//     const visible_object *vo = &current->vo;

//     printf("Record:\n");
//     printf("  Type: ");
//     switch (vo->type) {
//       case UNKNOWN:
//         printf("UNKNOWN\n");
//         break;
//       case MUTEX:
//         printf("MUTEX\n");
//         printf("  Location: %p\n", vo->location);
//         printf("  Mutex State: ");
//         switch (vo->mut_state) {
//           case UNINITIALIZED: printf("UNINITIALIZED\n"); break;
//           case UNLOCKED: printf("UNLOCKED\n"); break;
//           case LOCKED: printf("LOCKED\n"); break;
//           case DESTROYED: printf("DESTROYED\n"); break;
//           default: printf("UNKNOWN STATE\n"); break;
//         }
//         break;
//       case SEMAPHORE:
//         printf("SEMAPHORE\n");
//         printf("  Location: %p\n", vo->location);
//         printf("  Count: %d\n", vo->sem_state.count);
//         break;
//       case CONDITION_VARIABLE:
//         printf("CONDITION VARIABLE\n");
//         printf("  Location: %p\n", vo->location);
//         printf("  Status: ");
//         switch (vo->cond_state.status) {
//           case CV_UNINITIALIZED: printf("UNINITIALIZED\n"); break;
//           case CV_INITIALIZED: printf("INITIALIZED\n"); break;
//           case CV_WAITING: printf("WAITING\n"); break;
//           case CV_SIGNALED: printf("SIGNALED\n"); break;
//           case CV_PREWAITING: printf("TRANSITIONAL\n"); break;
//           default: printf("UNKNOWN STATUS\n"); break;
//         }
//         printf("  Waiting Thread: %p\n", (void *)vo->cond_state.interacting_thread);
//         printf("  Associated Mutex: %p\n", (void *)vo->cond_state.associated_mutex);
//         printf("  Waiting Count: %d\n", vo->cond_state.count);
//         break;
//       case THREAD:
//         printf("THREAD\n");
//         printf("  Thread Descriptor: %p\n", (void *)vo->thrd_state.pthread_desc);
//         printf("  Runner ID: %u\n", vo->thrd_state.id);
//         printf("  Status: ");
//         switch (vo->thrd_state.status) {
//           case ALIVE: printf("ALIVE\n"); break;
//           case EXITED: printf("EXITED\n"); break;
//           default: printf("UNKNOWN STATUS\n"); break;
//         }
//         break;
//       default:
//         printf("INVALID TYPE\n");
//         break;
//     }

//     current = current->next;
//   }
// }


void notify_template_thread() { sem_post(&dmtcp_restart_sem); }

bool is_in_restart_mode(void) {
  enum libmcmini_mode mode = get_current_mode();
  return mode == DMTCP_RESTART_INTO_BRANCH ||
         mode == DMTCP_RESTART_INTO_TEMPLATE;
}
enum libmcmini_mode get_current_mode() { return atomic_load(&libmcmini_mode); }
void set_current_mode(enum libmcmini_mode new_mode) {
  atomic_store(&libmcmini_mode, new_mode);
}
