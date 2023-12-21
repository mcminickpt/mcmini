#include "mcmini/mcmini.hpp"

#define _XOPEN_SOURCE_EXTENDED 1

#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>

class OurInterface {
 public:
  virtual ~OurInterface();
};

class MyPluginImpl : public OurInterface {
 public:
  int c = 0;

  MyPluginImpl(int c) : c(c) {}
};

// Inside McMini process.... the id table is known!
OurInterface *transition_from_stream_contents(std::istream &istream) {
  // Read transition id
  int transition_id;
  istream >> transition_id;

  // Jump to the appropriate handler based on the id -->
  // Look in id table at registration time --> they pointed us to it when we
  // loaded them!!
  return nullptr;
}

void serialize_transition(MyPluginImpl &impl, std::ostream &o) {
  o << "Ahhhhhhhhhhh + " << impl.c << std::endl;
}

// Inside libmcmini.so

// std::istream for each

// We somehow have to write the type... where do we get this though ?? -> at
// registration time ??

std::istream transition_return_inside_wrapper(
    std::ostream &,
    int id_of_transition_at_registration_time);  /// Constructor of dylib can
                                                 /// load this...

class G {};

class MyWrapperWriteStruct {};

class MyTransition {
 public:
  MyTransition(std::istream &);
};

template <typename T>
std::unique_ptr<T> when_wrapper_is_hit(std::ostream &);

template <typename T>
void serialize_to(const T &, std::ostream &);

extern "C" void test(std::ostream &stream, ...) {
  va_list list;

  va_start(list, stream);

  auto j = va_arg(list, MyWrapperWriteStruct *);

  std::cout << (void *)j << std::endl;

  va_end(list);
}

// Serialize information needed to build a transition
void serialize_transition_hit_inside_wrapper(volatile void *cntx,
                                             std::ostream &o) {
  // libmcmini.so --> what is the id for the transition id for __FUNCTION__
  //
  //
  //
  o << 10 << std::endl;
  o << __func__ << std::endl;
}

class ModelCheckingContext {
 public:
  void register_transition_type();
};

extern "C" void mcmini_register_plugin(ModelCheckingContext &cntx) {}

int main(int argc, char **argv) {
  if (argc > 1) {
    char **cur_arg = &argv[1];

    char buf[1000];
    buf[sizeof(buf) - 1] = '\0';
    // We add ours to the end of any PRELOAD of the target application.
    snprintf(buf, sizeof buf, "%s:%s/libmcmini.so",
             (getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : ""),
             dirname(argv[0]));
    setenv("LD_PRELOAD", buf, 1);

    if (fork() == 0) {
      execvp(cur_arg[0], cur_arg);
    } else {
      std::cout << "parent!!" << std::endl;
    }
  } else {
    std::cout << "Droid is ready!" << std::endl;
  }
}