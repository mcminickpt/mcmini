#pragma once

#include <string>

namespace real_world {

/**
 * @brief A handle to FIFO/named pipe.
 */
struct fifo {
 private:
  int fd = -1;
  std::string name;

 public:
  fifo(const std::string&);
  fifo(const fifo&) = delete;
  fifo(fifo&&) = default;
  fifo& operator=(const fifo&) = delete;
  fifo& operator=(fifo&&) = default;
  ~fifo();

  size_t read(void* buf, size_t size) const;
  size_t write(const void* buf, size_t size) const;

  template <typename T>
  size_t read(T* val) const {
    return read(val, sizeof(T));
  }
  template <typename T>
  size_t write(const T* val) const {
    return write(val, sizeof(T));
  }
};

}  // namespace real_world
