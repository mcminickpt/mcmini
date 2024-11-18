#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <thread>

typedef enum visible_object_type {
  MUTEX,
  SEMAPHORE,
  CONDITION_VARIABLE,
} visible_object_type;

typedef enum mutex_state {
  UNINITIALIZED,
  UNLOCKED,
  LOCKED,
  DESTROYED
} mutex_state;

typedef struct visible_object {
  visible_object_type type;
  void *location;
  union {
    mutex_state mut_state;
  };
} visible_object;

static const char *fifo_name = "/tmp/our_fifo";

void reads_the_fifo() {
  int fd = open(fifo_name, O_RDONLY);
  visible_object received_value;
  while (read(fd, &received_value, sizeof(visible_object))) {
    std::cout << "Received: " << received_value.location << std::endl;
  }
  close(fd);
}

void writes_to_the_fifo() {
  int fd = open(fifo_name, O_WRONLY);

  visible_object recorded_objects[10];
  recorded_objects[4].location = (void *)0x400;
  recorded_objects[7].location = (void *)0x900;
  for (int i = 0; i < 10; i++) {
    write(fd, &recorded_objects[i], sizeof(visible_object));
  }
  close(fd);
}

int main(int argc, const char **argv) {
  int fifo_fd = -1;
  if ((fifo_fd = mkfifo(fifo_name, S_IRUSR | S_IWUSR)) != 0 &&
      errno != EEXIST) {
    std::cerr << "Error (mkfifo failed): " << strerror(errno) << std::endl;
    return EXIT_FAILURE;
  }

  std::thread reader_thread(&reads_the_fifo);
  std::thread writer_thread(&writes_to_the_fifo);
  reader_thread.join();
  writer_thread.join();

  if (fifo_fd != -1) close(fifo_fd);
  return 0;
}
