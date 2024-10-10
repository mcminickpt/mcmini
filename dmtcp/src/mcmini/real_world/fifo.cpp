#include "mcmini/real_world/fifo.hpp"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace real_world;

fifo::fifo(const std::string& name) : name(name) {
  if (mkfifo(name.c_str(), 0666) != 0) {
    if (errno == EEXIST) {
      // Remove the fifo and re-attempt to create it
      unlink(name.c_str());
      if (mkfifo(name.c_str(), 0666) != 0) {
        std::perror("mkfifo");
        std::exit(EXIT_FAILURE);
      }
    } else {
      std::perror("mkfifo");
      std::exit(EXIT_FAILURE);
    }
  }
  fd = open(name.c_str(), O_RDONLY);
}

size_t fifo::read(void* buf, size_t size) const {
  return ::read(fd, buf, size);
}

size_t fifo::write(const void* buf, size_t size) const {
  return ::write(fd, buf, size);
}

fifo::~fifo() {
  if (fd != -1) close(fd);
  unlink(name.c_str());
}
