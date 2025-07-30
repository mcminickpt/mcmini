#include <pthread.h>

namespace mcmini {
class RWLock {
 public:
  // According to the man page of `pthread_rwlockattr_setkind_np(3)`:
  //
  // """
  //  PTHREAD_RWLOCK_PREFER_READER_NP
  //           This is the default.  A thread may hold multiple read
  //           locks; that is, read locks are recursive.
  // """
  // Therefore, the RWLock is implicitly safe to use recursively either strictly
  // as a reader or strictly as a writer.
  RWLock() { pthread_rwlock_init(&lock_, nullptr); }

  ~RWLock() { pthread_rwlock_destroy(&lock_); }
  RWLock(const RWLock&) = delete;
  RWLock& operator=(const RWLock&) = delete;

  void lock_read() { pthread_rwlock_rdlock(&lock_); }
  void unlock_read() { pthread_rwlock_unlock(&lock_); }
  void lock_write() { pthread_rwlock_wrlock(&lock_); }
  void unlock_write() { pthread_rwlock_unlock(&lock_); }

  class ReadGuard {
   public:
    explicit ReadGuard(RWLock& rwlock) : rwlock_(rwlock) {
      rwlock_.lock_read();
    }
    ~ReadGuard() { rwlock_.unlock_read(); }
    ReadGuard(const ReadGuard&) = delete;
    ReadGuard& operator=(const ReadGuard&) = delete;

   private:
    RWLock& rwlock_;
  };

  class WriteGuard {
   public:
    explicit WriteGuard(RWLock& rwlock) : rwlock_(rwlock) {
      rwlock_.lock_write();
    }

    ~WriteGuard() { rwlock_.unlock_write(); }
    WriteGuard(const WriteGuard&) = delete;
    WriteGuard& operator=(const WriteGuard&) = delete;

   private:
    RWLock& rwlock_;
  };

 private:
  pthread_rwlock_t lock_;
};
}  // namespace mcmini
