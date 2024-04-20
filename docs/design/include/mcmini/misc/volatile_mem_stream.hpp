#pragma once

#include <iostream>
#include <streambuf>

#include "mcmini/real_world/shm.hpp"

struct volatile_mem_stream : public std::streambuf {
 private:
  char *volatile_cache;
  size_t rw_region_size;
  volatile char *rw_region;

 public:
  volatile_mem_stream(const real_world::shared_memory_region &read_write_region)
      : volatile_mem_stream(read_write_region.get(), read_write_region.size()) {
    reset();
  }
  volatile_mem_stream(volatile void *rw_start, size_t rw_size)
      : rw_region(static_cast<volatile char *>(rw_start)),
        rw_region_size(rw_size),
        volatile_cache(new char[rw_size]) {
    reset();
  }

  void reset() {
    setg(volatile_cache, volatile_cache, volatile_cache + rw_region_size);
    setp(volatile_cache, volatile_cache + rw_region_size);
  }

  ~volatile_mem_stream() { delete[] volatile_cache; }

 protected:
  int_type underflow() override {
    std::copy(rw_region, rw_region + rw_region_size, volatile_cache);
    setg(volatile_cache, volatile_cache, volatile_cache + rw_region_size);
    return this->gptr() != this->egptr() ? traits_type::to_int_type(*gptr())
                                         : traits_type::eof();
  }
  int_type overflow(int_type ch = traits_type::eof()) override {
    std::copy(volatile_cache, volatile_cache + rw_region_size, rw_region);
    setp(volatile_cache, volatile_cache + rw_region_size);
    return this->pptr() != this->epptr() ? ch : traits_type::eof();
  }
  int sync() override {
    std::copy(volatile_cache, volatile_cache + rw_region_size, rw_region);
    return std::streambuf::sync();
  }
};