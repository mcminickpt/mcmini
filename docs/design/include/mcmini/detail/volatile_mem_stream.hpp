#pragma once

#include <streambuf>

#include "mcmini/real_world/shm.hpp"

struct volatile_mem_stream : public std::streambuf {
 private:
  const real_world::shared_memory_region *read_write_region;
  char *volatile_cache;

 public:
  volatile_mem_stream(const real_world::shared_memory_region *read_write_region)
      : read_write_region(read_write_region),
        volatile_cache(new char[read_write_region->size()]) {
    reset();
  }

  void reset() {
    std::copy(read_write_region->byte_stream(),
              read_write_region->byte_stream() + read_write_region->size(),
              volatile_cache);
    setg(volatile_cache, volatile_cache,
         volatile_cache + read_write_region->size());
    setp(volatile_cache, volatile_cache + read_write_region->size());
  }

  ~volatile_mem_stream() { delete[] volatile_cache; }

 protected:
  std::streamsize xsputn(const char *s, std::streamsize n) override {
    std::ptrdiff_t n_bounded = n;  // std::min(egptr() - gptr(), n);
    std::copy(s, s + n_bounded, pptr());
    pbump(n_bounded); /* Move the write head */
    return n_bounded;
  }
  std::streamsize xsgetn(char *s, std::streamsize n) override {
    std::ptrdiff_t n_bounded = n;  // std::min(egptr() - gptr(), n);
    std::copy(gptr(), gptr() + n_bounded, s);
    gbump(n_bounded); /* Move the read head */
    return n_bounded;
  }
  int sync() override {
    std::copy(volatile_cache, volatile_cache + read_write_region->size(),
              read_write_region->byte_stream());
    return std::streambuf::sync();
  }
};