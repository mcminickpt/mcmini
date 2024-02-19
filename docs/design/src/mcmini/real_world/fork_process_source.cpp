#include "mcmini/real_world/process/fork_process_source.hpp"

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/real_world/process/local_linux_process.hpp"

using namespace real_world;

std::unique_ptr<process> fork_process_source::make_new_process() {
  // const_cast<> is needed to call the C-functions here. A new/delete
  // or malloc/free _could be_ needed, we'd need to check the man page. As long
  // as the char * is not actually modified, this is OK and the best way
  // to interface with the C library routines

  setup_ld_preload();

  pid_t child_pid = fork();
  if (child_pid == -1) {
    perror("fork");
    return nullptr;  // Handle fork() failing
  }

  if (child_pid == 0) {
    // TODO: Add additional arguments here if needed
    char* args[] = {const_cast<char*>(this->target_program.c_str()), NULL};

    std::cerr << "About to exec with libmcmini.so loaded! Attempting to run "
              << this->target_program.c_str() << std::endl;
    execvp(this->target_program.c_str(), args);

    perror("execvp");  // Handle execvp error here
    exit(EXIT_FAILURE);
  } else {
    int status;
    waitpid(child_pid, &status, 0);  // Wait for the child to exit
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
      // Handle execvp failing in the fork()-ed child
      return nullptr;
    }
  }

  return extensions::make_unique<local_linux_process>(child_pid);
}

void fork_process_source::setup_ld_preload() {
  char buf[1000];
  buf[sizeof(buf) - 1] = '\0';
  snprintf(buf, sizeof buf, "%s:%s/libmcmini.so",
           (getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : ""),
           dirname(const_cast<char*>(this->target_program.c_str())));
  setenv("LD_PRELOAD", buf, 1);
}