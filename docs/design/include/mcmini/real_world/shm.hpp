#pragma once

#include <string>

namespace real_world {

/**
 * @brief A handle to a region of shared memory shared between the McMini
 * process and the child processes McMini spawns during exploration.
 *
 * A `shared_memory_region` is a light-weight wrapping around the functions
 * `shm_open()` and `mmap()` to map the shared memory file into the particular
 * region.
 */
struct shared_memory_region {
 public:
  /**
   * @brief Create a new shared memory
   */
  shared_memory_region(const std::string& shm_file_name, size_t region_size);
  virtual ~shared_memory_region();

  // Ownership can only be passed -- prevent copying to prevent
  shared_memory_region(const shared_memory_region&) = delete;
  shared_memory_region(shared_memory_region&&) = default;
  shared_memory_region& operator=(const shared_memory_region&) = delete;
  shared_memory_region& operator=(shared_memory_region&&) = default;

  volatile void* get() const { return this->shm_mmap_region; }
  volatile void* contents() const { return get(); }

  template <typename T>
  volatile T* as_stream_of() const {
    return static_cast<volatile T*>(shm_mmap_region);
  }
  volatile char* byte_stream() const { return as_stream_of<char>(); }
  size_t size() const { return this->region_size; }

 private:
  std::string shm_file_name;
  volatile void* shm_mmap_region;
  size_t region_size;
};

};  // namespace real_world