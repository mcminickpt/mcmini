#pragma once

#include <iostream>
#include <type_traits>

namespace real_world {

/**
 * @brief A location for threads to read and write information when
 * communicating with `libmcmini.so`.
 *
 * Threads and operations managed in `libmcmini.so` communicate with McMini via
 * the coordinator (see coordinator.hpp). Communication occurs over some shared
 * medium between the McMini process and the ephmeral processes McMini spawns as
 * it continues exploration during model checking. Threads communicate with
 * McMini via a mailbox, which the coordinator accesses as part of its.
 */
struct runner_mailbox_stream {
 private:
  mutable std::iostream msg_stream;

 public:
  runner_mailbox_stream(runner_mailbox_stream &&) = default;
  runner_mailbox_stream(std::streambuf *sb) : msg_stream(sb) {}

  template <typename T>
  void read(T *t) const {
    static_assert(
        std::is_trivially_copyable<T>::value,
        "Only trivially-copyable types (i.e. those for which a `std::memcpy` "
        "is well-defined) are permitted in thread mailboxes.");
    msg_stream.read(reinterpret_cast<char *>(t), sizeof(T));
  }

  template <typename T>
  void write(T *t) {
    static_assert(
        std::is_trivially_copyable<T>::value,
        "Only trivially-copyable types (i.e. those for which a `std::memcpy` "
        "is well-defined) are permitted in thread mailboxes.");
    msg_stream.write(reinterpret_cast<char *>(t), sizeof(T));
    msg_stream.flush();
  }

  ~runner_mailbox_stream() { msg_stream.flush(); }
};
}  // namespace real_world