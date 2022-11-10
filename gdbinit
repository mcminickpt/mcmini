# Configure GDB in a reasonable way
set breakpoint pending on
set pagination off
# GDB will track both parent and child
set detach-on-fork off
set print pretty
set print address off
# Stop if about to exit:
break _exit
set variable $bpnum_exit = $bpnum
# In McMini, parent sends SIGUSR1 to child on exit.
handle SIGUSR1 nostop noprint pass
handle SIGUSR2 nostop noprint pass
# Allow the other inferior to continue to execute if not at breakpoint
set schedule-multiple on
## Optional for additional modes for threads/inferios:
## SEE: https://stackoverflow.com/questions/27140941/preventing-debugging-session-from-pausing-after-each-inferior-exits
# set non-stop on
# set target-async on

# FIXME:
## Newer GDBs can use "tui disable" to undo "layout src"
## We should test if "tui disable" works, and then use it.

source gdbinit_commands.py

break main
run

break execvp
continue

break 'mcmini_main()'
continue

## WE WANT TO DO:  break __real_sem_init
## Unfortunately, it's a macro.  So, it's tricky to set breakpoint on it.
## In next version, let's make it a function,
##   with a local variable:
##  __real_sem_init(...) {
##    static typeof(&sem_init) sem_init_ptr = NULL;
##    if (sem_init_ptr == NULL) { dlsym(RTLD_NEXT, "sem_init"); }
##    (*sem_init_ptr)(...);
##  }
#### This variation will actually put a breakpoint on sem_init, since
#### *sem_init_ptr and sem_init are the same address in memory.  :-(
## We need to break at sem_init only for debugging McMini
### FIXME:  If I comment these lines out, I see a bug when doing:
###    (gdb) finishTrace
###    (gdb) nextTrace
 # break mc_load_intercepted_symbol_addresses
 # continue
 # finish
 # break *&sem_init_ptr
 ### The above works with gcc-4.8.  Higher versions of gcc need following:
 ### See abvoe for how to rewrite sem_init_ptr, etc., to avoid this issue.
 ## break *(long *)&sem_init_ptr
 ## break *(void **)&sem_init_ptr
 # continue
 # disable $bpnum

# break 'mc_initialize_trace_sleep_list()'
# break *sem_init_ptr

# break fork
# continue
# continue

# Continue through 'fork' and into target (child) process
continue

## Python nextTransition will set this:
## break mc_new_trace_at_current_state()
## break mc_shared_cv_wait_for_scheduler
# break thread_await_scheduler_for_thread_start_transition
# NOT NEEDED:
#   set follow-fork-mode child

# Print Python-based GDB commands:
help user-defined
echo \n\ \ *** Type 'mcmini help' for usage. ***\n\n
