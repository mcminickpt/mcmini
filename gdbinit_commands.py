# Nice tutorial:
# https://interrupt.memfault.com/blog/automate-debugging-with-gdb-python-api

## NOTE:  Inside gdb, you can test Python commands.  For example:
##   (gdb) python gdb.execute("bt")
##   (gdb) python print(gdb.parse_and_eval("main"))
## INTERACTIVE DEBUGGING:
##   (gdb) python-interactive
##   (gdb) nextTransitionCmd().invoke("3",True)  # Invoke it with count of 3

## FIXME:
## Use 'gdb.Breakpoint' instead of 'gdb.execute("break ..."),
##   so that we can easily temporarily disable all breakpoints.
## EXAMPE USAGE:
##   bkptMain = gdb.Breakpoint("main")
##   bkptMain.silent = True
##   bkptMain.enabled = False
##   bkptMain.thread = <Thread>
##   gdb.breakpoints()

## FIXME:
## mcmini nextTrace <count> fails, when using optional <count> argument.

## FIXME:
## Newer GDBs can use "tui disable" to undo "layout src"
## We should test if "tui disable" works, and then turn on "layout src"
##   after 'nextTransition' or when in lower half.
## As a stopgap, could use 'shell reset', 'shell setenv TERM=vt100'
##   followed by 'mcmini printTransitions' and then 'layout src'.

## TODO:
## 1. mcmini printTransitions => McMini print:  Number the transitions,
##      and include '==>' to highlight the current transition.
##    EXAMPLE:
##      7. thread 2: pthread_mutex_unlock(1)
##      8.==> thread 2: sem_post(3)
##      9. thread 2: sem_wait(2) (awake -> asleep)
## 2. Add global variable 'transitionId' within a trace in C code, to
##    replace GDB variable,.'$transitionId'.  In this way, if the user
##    repeatedly executes GDB 'next' beyond the next transition, the
##    GDB API will correctly show the new transitionId
## 3. 'assert()' and '_exit()' should be handled gracefully. (Especially assert)
##    As a stopgap, in case McMini crashes, just print every traceId before that
## 4. Whan McMini stops at "bad" trace (e.g., deadlock), print traceId.
## 5. In WSL, the arguments of the target program are getting lost. WHY?
##    As a result, producer_consumer doesn't recognize its '--quiet' flag.
## 6. I'm not a fan of using 'operator' widely for the same reason that
##    I'm not a fan of using global variables widely.  It becomes very
##    difficult to read the code locally, since absolutely any use
##    of '[]' or '()' might be an operator with unknown semantics.
##    When large projects use operators, they have detailed documentation
##    at the beginning of the code, to warn code readers that the
##    the code does not follow the "natural semantics" for C++,
##    and anybody using a debugger must first study the operator implementation.
##    One can make a (weak) case for '==' and '=', by saying that they
##    normally do a shallow compare/copy, and we are changing the
##    semantics to do a deep compare/copy.  But '()' and '[]' is weird.

## FIXME:
##  We could set thread name of the McMini scheduler thread:
##    prctl (PR_SET_NAME, "MCMINI_INTERNAL", 0, 0, 0)
##  to distinguish it from the user threads.

## FIXME:
##  Bug:  If we continue to do 'mcmini forward' until end of trace, it crashes.

# ===========================================================
# Print statistics

transitionId = 0
def print_mcmini_stats():
  global transitionId
  print("*** trace: " + str(gdb.parse_and_eval("traceId")) + "; " +
        "transition: " + str(transitionId) + "; "
        "thread: " + str(gdb.selected_inferior().num) + "." +
                     str(gdb.selected_thread().num) +
        "\n")

def print_user_frames_in_stack():
  if gdb.selected_inferior().num == 1:
    print("Internal error: print_user_frames_in_stack called " +
                           "outside user program\n")
  level = 0
  mcmini_num_frame_levels = 0
  frame = gdb.newest_frame()
  while True:
    frame = frame.older()
    if not frame:
      break
    level += 1
    if (frame.name() and
         (frame.name().startswith("mc_") or
          frame.name().startswith("__real_") or
          frame.name() == "thread_await_scheduler_for_thread_start_transition")
        and 
         (frame.name() != "mc_thread_routine_wrapper" or
          frame.newer().name() ==
                         "thread_await_scheduler_for_thread_start_transition")
       ):
      mcmini_num_frame_levels = level
      frame.older().select() # Keep setting older frame as the user frame
  gdb.execute("bt " + str(- (level - mcmini_num_frame_levels)))
def find_call_frame(name):
  frame = gdb.newest_frame()
  while frame:
    if frame.name() == name:
      return frame
    frame = frame.older()
  return None

# ===========================================================
# Set up breakpoint utilities

def continue_until(function, thread_id=None):
  ## gdb 7.6 doesn't seem to implement "temporary" optional argument.
  ## Optional arguments:  internal=False, temporary=True
  ## bkpt = gdb.Breakpoint("main", gdb.BP_BREAKPOINT, gdb.WP_WRITE.
  ##                       False, True)
  ## while bkpt.is_valid():
  ##   gdb.execute("continue")
  # Optional argument:  internal=True
  bkpt = gdb.Breakpoint(function, gdb.BP_BREAKPOINT, gdb.WP_WRITE, True)
  bkpt.silent = True
  if thread_id:
    ckpt.thread = thread_id
  while bkpt.hit_count == 0:
    gdb.execute("continue")
  bkpt.delete()

def continue_beyond(function):
  # Last two arguments:  internal: False; temporary: True
  ## This variant fails with "bad syntax" in some GDB/Python versions.
  # bkpt = gdb.Breakpoint("main", gdb.BP_BREAKPOINT, gdb.WP_WRITE. False, True)
  bkpt = gdb.Breakpoint("main", gdb.BP_BREAKPOINT, gdb.WP_WRITE)
  bkpt.silent = True
  while bkpt.isValid:
    gdb.execute("continue")
  # Continue until pre-existing breakpoint
  gdb.execute("continue")

# gdb.Breakpoint.stop() can be defined to do anything arbitrary when
#                       reaching the breakpoint.

# ===========================================================
# Set up McMini commands

class mcminiPrefixCmd(gdb.Command):
  """mcmini <TAB> : show all mcmini commands"""
  def __init__(self):
    super(mcminiPrefixCmd, self).__init__(
        "mcmini", gdb.COMMAND_USER, gdb.COMPLETE_COMMAND, prefix=True
    )
mcminiPrefixCmd()

mcminiHelpString=(
"""\
********************************************************************************
* Do "mcmini <TAB>" or "help user-defined" to list McMini commands.            *
* TERMINOLOGY AND USAGE:                                                       *
*   A 'trace' is a single execution of the target program.                     *
*   A 'transition' is a thread operation.                                      *
*   The usual GDB commands (next, step, finish, etc.) and TAB-completion work. *
*   But a command like 'mcmini forward' will skip forward to just before       *
*   the next transition (the next thread operation)  'mcmini back' also exists.*
*                                                                              *
* OTHER HINTS:                                                                 *
* Consider using ctrl-Xa ('ctrl-X' and 'x') to toggle source display on or off.*
*   In 'ctrl-Xa' mode in WSL, you may need to type 'ctrl-L' to refresh screen. *
* Use GDB commands 'up' and 'down' to view other call frames.                  *
* But in source display, cursor keys only browse the source code, and          *
*   you cannot scroll back to see previous command output.                     *
* Turn off source display to re-execute commands or to scroll backward.        *
*                                                                              *
* Note that for certain technical reasons in the implementation of McMini,     *
* certain thread functions (e.g., sem_wait, pthread_cond_wait)                 *
* will appear to be executed twice when calling 'mcmini forward'.              *
*                                                                              *
* Note that 'mcmini print' can sometimes print future transitions, even before *
* they have been reached.                                                      '
*                                                                              *
* For details of 'mcmini' commands, type 'help user-defined'.                  *
********************************************************************************
"""
)

class helpCmd(gdb.Command):
  """Prints the transitions currently on the stack"""
  def __init__(self):
    super(helpCmd, self).__init__(
        "mcmini help", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    global mcminiHelpString
    print(mcminiHelpString)
helpCmd()

class printTransitionsCmd(gdb.Command):
  """Prints the transitions currently on the stack"""
  def __init__(self):
    super(printTransitionsCmd, self).__init__(
        "mcmini printTransitions", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
   current_inferior = gdb.selected_inferior().num
   gdb.execute("inferior 1")  # inferior 1 is scheduler process
   transition_stack = gdb.parse_and_eval("programState->printTransitionStack()")
   print(transition_stack)
   gdb.execute("inferior " + str(current_inferior))
printTransitionsCmd()

class forwardCmd(gdb.Command):
  """Execute until next transition; Accepts optional <count> arg"""
  def __init__(self):
    super(forwardCmd, self).__init__(
        "mcmini forward", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    global transitionId
    args = args.split()
    iterations = int(args[0]) if args and args[0].isdigit() else 1
    if iterations > 1:
      gdb.execute("mcmini forward " + str(iterations-1) + " quiet")
    # else iterations == 1
    if gdb.selected_inferior().num == 1:
      print("GDB is in scheduler, not target process:" +
            "  Can't go to next transition\n")
      return
    # GDB based on Red Hat use debuginfo files.  This will suppress
    # the warning.  However, this GDB command fails on Debian-based distros,
    # and anyway, no warning about missing debug files issued.
    try:
      gdb.execute("set build-id-verbose 0")
    except:
      pass
    continue_until("mc_shared_cv_wait_for_scheduler")
    transitionId += 1
    if "quiet" not in args:
      print_user_frames_in_stack()
      print_mcmini_stats()
forwardCmd()

class backCmd(gdb.Command):
  """Go back one transition of current trace, by re-executing"""
  def __init__(self):
    super(backCmd, self).__init__(
        "mcmini back", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    global transitionId
    if gdb.selected_inferior().num == 1:
      print("GDB is in scheduler, not target process:" +
            "  Can't go to previous transition\n")
      return
    iterationsForward = transitionId - 1
    gdb.execute("mcmini finishTrace quiet")
    gdb.execute("set rerunCurrentTraceForDebugger = 1")
    gdb.execute("mcmini nextTrace quiet")
    gdb.execute("mcmini forward " + str(iterationsForward) + " quiet")
    gdb.execute("set rerunCurrentTraceForDebugger = 0")
    print("DEBUGGING: " + "mcmini forward " + str(iterationsForward) + " quiet")
    print_user_frames_in_stack()
    print_mcmini_stats()
backCmd()

class whereCmd(gdb.Command):
  """Execute where, while hiding McMini internal call framces"""
  def __init__(self):
    super(whereCmd, self).__init__(
        "mcmini where", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    print_user_frames_in_stack()
whereCmd()

class finishTraceCmd(gdb.Command):
  """Execute until next trace"""
  breakpoint_for_next_transition = None
  def __init__(self):
    super(finishTraceCmd, self).__init__(
        "mcmini finishTrace", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    global transitionId
    args = args.split()
    if gdb.selected_inferior().num == 1:
      print("GDB is in scheduler process, not target:\n" +
            "  Try 'mcmini nextTrace' to go to next trace\n")
      return
    gdb.execute("disable " + str(gdb.parse_and_eval("$bpnum_exit")))
    # This continues until inferior reaches _exit and exits.
    gdb.execute("continue")
    gdb.execute("inferior 1")  # inferior 1 is scheduler process
    if (gdb.selected_frame().name() == "waitpid" or
        "__GI___wait4"):
      # Wait for zombie child, or we hit GDB bug.
      gdb.execute("finish")
    gdb.execute("enable " + str(gdb.parse_and_eval("$bpnum_exit")))
    # If the target is still in the constructor mcmini_main, then
    #   finishTrace might take us only to the next breakpoint in the target.
    #   So, we detect this and recursively call finishTrace once more.
    if len(gdb.inferiors()) > 1:
      target = [inferior for inferior in gdb.inferiors() if inferior.num > 1][0]
      gdb.execute("inferior " + str(int(target.num)))
      gdb.execute("mcmini finishTrace quiet")
    if "quiet" not in args:
      print_mcmini_stats()
    transitionId = 0
finishTraceCmd()

class nextTraceCmd(gdb.Command):
  """Execute to next trace; Accepts optional <count> arg"""
  def __init__(self):
    super(nextTraceCmd, self).__init__(
        "mcmini nextTrace", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    args = args.split()
    targetTraceId = int(args[0]) if args and args[0].isdigit() else 1
    if gdb.selected_inferior().num != 1:
      # FIXME:  We should automatically call 'mcmini finishTrace' for user.
      print("GDB is in target process, not scheduler:\n" +
            "  Execute 'mcmini finishTrace' to return to scheduler\n")
      return
    bkptMain = [bkpt for bkpt in gdb.breakpoints() if bkpt.location=="main"][0]
    bkptMain.silent = True
    if targetTraceId > 1:
      for i in range(targetTraceId - 1):
        continue_until("mc_shared_cv_wait_for_scheduler")
        gdb.execute("mcmini finishTrace")
    # Now continue until in child process.
    continue_until("mc_shared_cv_wait_for_scheduler")
    # We should now be in the next child process.
    if "quiet" not in args:
      print_mcmini_stats()
nextTraceCmd()

class gotoTraceCmd(gdb.Command):
  """gotoTrace <traceId>: Execute until reaching trace <traceId>"""
  def __init__(self):
    super(gotoTraceCmd, self).__init__(
        "mcmini gotoTrace", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    if args.isdigit():
      iterations = int(args)
    else:
      print("Missing integer <traceId> argument\n")
      return
    if iterations <= int(gdb.parse_and_eval("traceId")):
      print("*** Current traceId: " + str(gdb.parse_and_eval("traceId")) +
            "; Can't go to earlier trace; skipping command\n")
      return
    if gdb.selected_frame().name() == "main":
      gdb.execute("continue")
    if gdb.selected_inferior().num != 1:
      gdb.execute("mcmini finishTrace quiet")
    # We should now have only the parent (inferior 1).
    gdb.execute("set detach-on-fork on")
    # Optional argument:  internal=False ## FIXME: Change to True when ready.
    # FIXME:  If fifth arg here, it says only 4 args.  But 5 args work above.
    bkpt = gdb.Breakpoint("mc_search_next_dpor_branch_with_initial_thread" +
                            "(unsigned long)",
                          gdb.BP_BREAKPOINT, gdb.WP_WRITE, True)
    bkpt.silent = True
    while int(gdb.parse_and_eval("traceId")) < iterations:
      gdb.execute("continue")
    bkpt.delete()
    gdb.execute("set detach-on-fork off")
    gdb.execute("mcmini nextTrace quiet")
    print_mcmini_stats()
gotoTraceCmd()

developerHelp = ("""\
Useful GDB commands:
  info inferiors
  inferior 1
  info threads
    [ Thread 1.1 is the thread of the scheduler process. ]
  thread 1.1
  info breakpoints
  where
""")

class developerModeCmd(gdb.Command):
  """For developers only.  Use at your own risk."""
  def __init__(self):
    super(developerModeCmd, self).__init__(
        "mcmini developerMode", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    gdb.execute("break mc_run_thread_to_next_visible_operation(unsigned long)")
    # current_inferior = gdb.selected_inferior().num
    # gdb.execute("inferior 1") # Set inferior to scheduler
    # scheduler_call_frame_fnc = "mc_shared_cv_wait_for_thread"
    # gdb.execute("break " + scheduler_call_frame_fnc)
    # This next command forces a GDB-internal bug in gdb-12.0
    # gdb.FinishBreakpoint().__init__(find_call_frame_fnc(scheduler_call_frame_fnc))
    # gdb.execute("inferior " + str(current_inferior))
    print("Breakpoint added to scheduler process.")
    gdb.execute("set print address on")
    print(developerHelp)
developerModeCmd()
