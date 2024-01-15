#include "mcmini/mcmini.hpp"

#include "mcmini/coordinator/coordinator.hpp"
#include "mcmini/detail/ddt.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state/detached_state.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor.hpp"
#include "mcmini/real_world/process/fork_process_source.hpp"

#define _XOPEN_SOURCE_EXTENDED 1

#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <utility>

void display_usage() {
  std::cout << "mcmini [options] <program>" << std::endl;
  std::exit(EXIT_FAILURE);
}

void do_model_checking(
    /* Pass arguments here or rearrange to configure the checker at
    runtime, e.g. to pick an algorithm, set a max depth, etc. */) {
  mcmini::model::detached_state state_of_program_at_main;
  mcmini::model::pending_transitions
      initial_first_steps;  // TODO: Create initializer or else add other
                            // methods
  /*
  TODO: Complete the initialization of the initial state here, i.e. a
  single thread "main" that is alive and then running the transition `t`
  */
  mcmini::model::program model_for_program_starting_at_main(
      std::move(state_of_program_at_main), std::move(initial_first_steps));

  // For "vanilla" model checking where we start at the beginning of the
  // program, a fork_process_source suffices (fork() + exec() brings us to the
  // beginning)
  auto process_source =
      mcmini::extensions::make_unique<mcmini::real_world::fork_process_source>(
          "demo");

  mcmini::coordinator coordinator(std::move(model_for_program_starting_at_main),
                                  std::move(process_source));

  std::unique_ptr<mcmini::model_checking::algorithm> classic_dpor_checker =
      mcmini::extensions::make_unique<mcmini::model_checking::classic_dpor>();

  classic_dpor_checker->verify_using(coordinator);
  std::cout << "Model checking completed!" << std::endl;
}

void do_model_checking_from_dmtcp_ckpt_file(std::string file_name) {
  mcmini::model::detached_state state_of_program_at_main;
  mcmini::model::pending_transitions
      initial_first_steps;  // TODO: Create initializer or else add other
                            // methods

  // // TODO: Complete the initialization of the initial state here, i.e. a
  // // single thread "main" that is alive and then running the transition

  {
      // Read that information from the linked list __inside the restarted
      // image__
      // while (! not all information read yet) {}
      // read(...);

      // auto state_of_some_object_in_the_ckpt_image = new mutex_state();
      // state_of_program_at_main.add_state_for();
  }

  {
    // initial_first_steps
    // Figure out what thread `N` is doing. This probably involves coordination
    // between libmcmini.so, libdmtcp.so, and the `mcmini` process
  }

  mcmini::model::program model_for_program_starting_at_main(
      std::move(state_of_program_at_main), std::move(initial_first_steps));

  // TODO: With a checkpoint restart, a fork_process_source doesn't suffice.
  // We'll need to create a different process source that can provide the
  // functionality we need to spawn new processes from the checkpoint image.
  auto process_source =
      mcmini::extensions::make_unique<mcmini::real_world::fork_process_source>(
          "ls");

  mcmini::coordinator coordinator(std::move(model_for_program_starting_at_main),
                                  std::move(process_source));

  std::unique_ptr<mcmini::model_checking::algorithm> classic_dpor_checker =
      mcmini::extensions::make_unique<mcmini::model_checking::classic_dpor>();

  classic_dpor_checker->verify_using(coordinator);

  std::cerr << "Model checking completed!" << std::endl;
}

struct Base {
 public:
  virtual ~Base() = default;
};
struct Test : public Base {
  void greater_than(Test *) {
    std::cerr << "Yello from test void" << std::endl;
  }
};

struct Test2 : public Test {
  void foo(Test *) { std::cerr << "Yello from test2 void" << std::endl; }
  int foobar(Test *) {
    std::cerr << "Yello from foobar" << std::endl;
    return 0;
  }

  int foobar2(Base *) {
    std::cerr << "Yello from foobar2" << std::endl;
    return 0;
  }
};

#include "mcmini/model/transition.hpp"
#include "mcmini/model/transitions/mutex/mutex_init.hpp"

// std::unordered_map<std::type_index,
//                    std::pair<stored_callback, opaque_callback>>
//     _interface_type_callback_function_table;

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <streambuf>

class my_stream_buff : public std::basic_streambuf<char> {
  std::streamsize write_pos = 0;
  std::streamsize read_pos = 0;

 public:
  volatile char *ptr;

  explicit my_stream_buff(size_t sz) : ptr(new volatile char[sz]) {}

  ~my_stream_buff() { delete ptr; }

 protected:
  std::streamsize xsputn(const char *s, std::streamsize n) override {
    std::copy(s + write_pos, s + write_pos + n, ptr);
    write_pos += n;
    return n;
  }
  std::streamsize xsgetn(char *s, std::streamsize n) override {
    std::copy(ptr + read_pos, ptr + read_pos + n, s);
    read_pos += n;
    return n;
  }

  int overflow(int c) override {
    // std::cerr
    return std::streambuf::overflow(c);
  }

  int underflow() override {
    // std::printf("Freom unerx");
    return 1;  // std::streambuf::underflow();
  }
};

class my_vol_stream_buf : public std::streambuf {
  static constexpr size_t total_size = 1024;

 public:
  volatile char *ptr;

  char buf[total_size];

  void refresh() {
    std::copy(ptr, ptr + total_size, buf);
    setg(buf, buf, buf + total_size);
    setp(buf, buf + total_size);
  }

  explicit my_vol_stream_buf(size_t sz) : ptr(new volatile char[sz]) {
    refresh();
  }

  explicit my_vol_stream_buf(volatile void *ptr)
      : ptr(static_cast<volatile char *>(ptr)) {
    refresh();
  }

  ~my_vol_stream_buf() { delete ptr; }

 protected:
  std::streamsize xsputn(const char *s, std::streamsize n) override {
    std::copy(s, s + n, pptr());
    pbump(n); /* Move the write head */
    return n;
  }
  std::streamsize xsgetn(char *s, std::streamsize n) override {
    std::copy(gptr(), gptr() + n, s);
    gbump(n); /* Move the read head */
    return n;
  }
  int sync() override {
    std::copy(buf, buf + total_size, ptr);
    return std::streambuf::sync();
  }
};

#include <sys/mman.h>

#include "mcmini/real_world/shm.hpp"

void signalasd(int sig) {}

void signal_handler(int signal) {}

void at_ex() { fprintf(stdout, "ATE XI\n"); }

#include "mcmini/detail/volatile_mem_stream.hpp"

int main(int argc, char **argv) {
  do_model_checking();

  // std::signal(SIGABRT, signal);
  // std::atexit(at_ex);
  // std::abort();

  Base b;
  Test t1;
  Test t2;
  Test2 t22;
  mcmini::real_world::shared_memory_region regions{"test", 100};

  mcmini::detail::volatile_mem_stream vms{&regions};

  // void *reg = mmap(NULL, 1000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, 0, 0);

  // if (reg == MAP_FAILED) {
  //   perror("mmap");
  //   exit(EXIT_FAILURE);
  // }
  // // munmap(a, 100);

  // volatile char *a = new volatile char[10];
  // volatile char *bb = new volatile char[10];

  // a[0] = 'A';
  // a[1] = 'B';
  // a[2] = 'C';
  // a[3] = 'D';
  // std::copy(a, a + 9, bb);

  // std::unique_ptr<volatile int> aaa;

  // delete a;
  // delete bb;

  auto *y = std::cerr.rdbuf(&vms);

  std::cerr << "abcdefghijklmnop" << std::endl;
  std::cerr << "qrst" << std::endl;
  std::cerr << "uvw";
  std::cerr.rdbuf(y);

  sleep(10);

  // my_vol_stream_buf buf2{100};
  // auto *y2 = std::cin.rdbuf(&buf2);
  // auto *y3 = std::cerr.rdbuf(&buf2);

  // int bbb = 0;
  // std::cerr << 10 << 'G' << 'E' << std::endl;
  // std::cin >> bbb;
  // // std::cout << buf2.buf[0] << "WHAT";

  // std::cout << bbb << " AD" << std::endl;

  // // std::cerr << 123 << 456 << 789 << std::endl;

  // // std::cout << bbb << " AD" << std::endl;

  // std::cin.rdbuf(y2);
  // std::cerr.rdbuf(y3);

  // mcmini::model::detached_state d;

  // mcmini::model::state::objid_t id =
  //     d.add_object(mcmini::model::objects::mutex_state::make(
  //         mcmini::model::objects::mutex_state::uninitialized));
  // mcmini::model::transitions::mutex_init mut{id};

  // std::cerr << mut.is_enabled_in(d) << std::endl;
  // std::cerr << (uintptr_t)(0x00) << std::endl;

  // mcmini::detail::double_dispatch_member_function_table<Base, void(void)>
  // ddt;
  // // // ddt;

  // ddt.register_dd_entry(&Test::greater_than);
  // ddt.register_dd_entry(&Test2::foo);

  // ddt.call(&t1, &t2);
  // ddt.call(&t1, &t22);

  // // // ddt.call(&t22, &t2);=

  // mcmini::detail::double_dispatch_member_function_table<Base, int(void)>
  // ddt1;
  // // ddt1;

  // ddt1.register_dd_entry(&Test2::foobar);
  // ddt1.register_dd_entry(&Test2::foobar2);

  // int h = ddt1.call(&b, &t22).value();
  // int gg = ddt1.call(&t2, &t22).value();

  // std::cerr << h << "\n";
}
