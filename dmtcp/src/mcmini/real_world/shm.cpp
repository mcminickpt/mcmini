#include "mcmini/real_world/shm.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace real_world;

shared_memory_region::shared_memory_region(const std::string &shm_file_name,
                                           size_t region_size)
    : shm_file_name(shm_file_name), region_size(region_size) {
  // This creates a file in /dev/shm/
  int const fd =
      shm_open(shm_file_name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    if (errno == EACCES) {
      std::fprintf(stderr,
                   "Shared memory region (%s) not owned by this process\n",
                   shm_file_name.c_str());
    } else {
      std::perror("shm_open");
    }
    return;
  }
  int const rc = ftruncate(fd, size());
  if (rc == -1) {
    std::perror("ftruncate");
    return;
  }
  void *handle =
      mmap(nullptr, size(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (handle == MAP_FAILED) {
    std::perror("mmap");
    return;
  }
  fsync(fd);
  close(fd);
  this->shm_mmap_region = handle;
}

shared_memory_region::~shared_memory_region() {
  // Note: mmap expects a `void*`. The field `shm_mmap_region` has a higher
  // cv-qualification than `void*` with `volatile void*` and thus a const_cast
  // is needed. While in general this would be considered unsafe, `mmap(2)` is
  // only concerned with the actual address value and will not attempt to access
  // the _contents_ of `shm_mmap_region` without the volatile qualification
  // (such an access is undefined behavior)
  int rc = shm_unlink(shm_file_name.c_str());
  if (rc == -1) {
    if (errno == EACCES) {
      std::fprintf(stderr,
                   "Shared memory region '%s' not owned by this "
                   "process\n ",
                   shm_file_name.c_str());
    } else {
      std::perror("shm_unlink");
    }
  }
  rc = munmap(const_cast<void *>(shm_mmap_region), size());
  if (rc == -1) {
    std::perror("munmap");
  }
}
