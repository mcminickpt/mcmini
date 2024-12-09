#include "mcmini/real_world/fifo.hpp"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

using namespace real_world;

fifo::fifo(const std::string& name) : name(name) {
  if (mkfifo(name.c_str(), S_IRUSR | S_IWUSR) != 0) {
    if (errno == EEXIST) {
      std::cerr << "The FIFO `" << name
                << "` already exists. We'll use the existing one";
    } else {
      std::perror("mkfifo");
      std::exit(EXIT_FAILURE);
    }
  }
  fd = open(name.c_str(), O_RDONLY);
  if (fd == -1) {
    std::perror("open");
    std::exit(EXIT_FAILURE);
  }
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
