// #include "mcmini/mcmini.hpp"

// #define _XOPEN_SOURCE_EXTENDED 1

// #include <libgen.h>
// #include <stdarg.h>
// #include <stdio.h>
// #include <unistd.h>

// #include <iostream>

// class OurInterface {
//  public:
//   virtual ~OurInterface();
// };

// class MyPluginImpl : public OurInterface {
//  public:
//   int c = 0;

//   MyPluginImpl(int c) : c(c) {}
// };

// // Inside McMini process.... the id table is known!
// OurInterface *transition_from_stream_contents(std::istream &istream) {
//   // Read transition id
//   int transition_id;
//   istream >> transition_id;

//   // Jump to the appropriate handler based on the id -->
//   // Look in id table at registration time --> they pointed us to it when we
//   // loaded them!!
//   return nullptr;
// }

// void serialize_transition(MyPluginImpl &impl, std::ostream &o) {
//   o << "Ahhhhhhhhhhh + " << impl.c << std::endl;
// }

// // Inside libmcmini.so

// // std::istream for each

// // We somehow have to write the type... where do we get this though ?? -> at
// // registration time ??

// std::istream transition_return_inside_wrapper(
//     std::ostream &,
//     int id_of_transition_at_registration_time);  /// Constructor of dylib can
//                                                  /// load this...

// class G {};

// class MyWrapperWriteStruct {};

// class MyTransition {
//  public:
//   MyTransition(std::istream &);
// };

// template <typename T>
// std::unique_ptr<T> when_wrapper_is_hit(std::ostream &);

// template <typename T>
// void serialize_to(const T &, std::ostream &);

// extern "C" void test(std::ostream &stream, ...) {
//   va_list list;

//   va_start(list, stream);

//   auto j = va_arg(list, MyWrapperWriteStruct *);

//   std::cout << (void *)j << std::endl;

//   va_end(list);
// }

// // Serialize information needed to build a transition
// void serialize_transition_hit_inside_wrapper(volatile void *cntx,
//                                              std::ostream &o) {
//   // libmcmini.so --> what is the id for the transition id for __FUNCTION__
//   //
//   //
//   //
//   o << 10 << std::endl;
//   o << __func__ << std::endl;
// }

// class ModelCheckingContext {
//  public:
//   void register_transition_type();
// };

// extern "C" void mcmini_register_plugin(ModelCheckingContext &cntx) {}

// int main(int argc, char **argv) {
//   if (argc > 1) {
//     char **cur_arg = &argv[1];

//     char buf[1000];
//     buf[sizeof(buf) - 1] = '\0';
//     // We add ours to the end of any PRELOAD of the target application.
//     snprintf(buf, sizeof buf, "%s:%s/libmcmini.so",
//              (getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : ""),
//              dirname(argv[0]));
//     setenv("LD_PRELOAD", buf, 1);

//     if (fork() == 0) {
//       execvp(cur_arg[0], cur_arg);
//     } else {
//       std::cout << "parent!!" << std::endl;
//     }
//   } else {
//     std::cout << "Droid is ready!" << std::endl;
//   }
// }

#include <iostream>
#include <vector>

class Foo {
 public:
  virtual ~Foo() = default;
  // Common interface for Foo and its subtypes...
};

class FooSub1 : public Foo {
  // Implementation specific to FooSub1...
};

class FooSub2 : public Foo {
  // Implementation specific to FooSub2...
};

// Generic template function
template <typename T>
void processFooSubtype(T* subtype, std::ostream& os) {
  // Default implementation or leave undefined
}

// Specializations for each subtype
template <>
void processFooSubtype<FooSub1>(FooSub1* subtype, std::ostream& os) {
  // Specific implementation for FooSub1
  os << "Foo1!" << std::endl;
}

template <>
void processFooSubtype<FooSub2>(FooSub2* subtype, std::ostream& os) {
  // Specific implementation for FooSub2
  os << "Foo2!" << std::endl;
}

int main() {
  std::vector<void (*)(Foo*, std::ostream&)> functions;

  // Storing function pointers
  functions.push_back(reinterpret_cast<void (*)(Foo*, std::ostream&)>(
      processFooSubtype<FooSub1>));
  functions.push_back(reinterpret_cast<void (*)(Foo*, std::ostream&)>(
      processFooSubtype<FooSub2>));

  // Example usage
  FooSub1 sub1;
  FooSub2 sub2;

  functions[0](&sub1, std::cout);  // Calls specialized function for FooSub1
  functions[1](&sub2, std::cout);  // Calls specialized function for FooSub2

  return 0;
}
