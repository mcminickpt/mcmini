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
  shared_memory_region(const std::string& shm_file_name, size_t region_size);
  shared_memory_region(const shared_memory_region&) = delete;
  shared_memory_region(shared_memory_region&&) = default;
  shared_memory_region& operator=(const shared_memory_region&) = delete;
  shared_memory_region& operator=(shared_memory_region&&) = default;
  ~shared_memory_region();

  volatile void* get() const { return this->shm_mmap_region; }
  volatile void* contents() const { return get(); }
  volatile void* off(off64_t offset) const {
    return static_cast<volatile void*>(
        static_cast<volatile char*>(shm_mmap_region) + offset);
  }
  template <typename T>
  volatile T* as() const {
    return static_cast<volatile T*>(shm_mmap_region);
  }
  template <typename T>
  volatile T* as_array_of(off64_t off_out = 0, off64_t off_in = 0) const {
    return static_cast<volatile T*>(off(off_in)) + off_out;
  }
  volatile char* byte_array(off64_t off = 0) const {
    return as_array_of<char>(off);
  }
  size_t size() const { return this->region_size; }

 private:
  std::string shm_file_name;
  volatile void* shm_mmap_region;
  size_t region_size;
};

};  // namespace real_world
