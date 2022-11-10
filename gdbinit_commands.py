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
##   bkptMain =gdb.Breakpoint("main")
##   bkptMain.silent = True
##   bkptMain.enabled = False

## FIXME:
## mcmini nextTrace <count> fails, when using optional <count> argument.

## FIXME:
## Newer GDBs can use "tui disable" to undo "layout src"
## We should test if "tui disable" works, and then turn on "layout src"
##   after 'nextTransition' or when in lower half.

## FIXME:
##  We could set thread name of the McMini scheduler thread:
##    prctl (PR_SET_NAME, "MCMINI_INTERNAL", 0, 0, 0)
##  to distinguish it from the user threads.

## FIXME:
##  Bug:  If we continue to do 'mcmini forward' until end of trace, it crashes.

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
  while (True):
    # print( "FRAME: " + frame.name()) 
    frame = frame.older()
    if not frame:
      break
    level += 1
    if frame.name() and (frame.name().startswith("mc_") or \
                         frame.name().startswith("__real_") or \
                         frame.name() ==
                         "thread_await_scheduler_for_thread_start_transition") \
       and \
       frame.name() != "mc_thread_routine_wrapper":
      mcmini_num_frame_levels = level
  gdb.execute("frame " + str(level - mcmini_num_frame_levels + 1))
  gdb.execute("bt " + str(- (level - mcmini_num_frame_levels)))
def find_call_frame(name):
  frame = gdb.newest_frame()
  while (frame):
    if (frame.name() == name):
      return frame
    frame = frame.older()
  return None

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
*   the next transition (the next thread operation).                           *
*                                                                              *
* OTHER HINTS:                                                                 *
* Consider using ctrl-Xa ('ctrl-X' and 'x') to toggle source display on or off.*
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
  breakpoint_for_next_transition = None
  def __init__(self):
    super(forwardCmd, self).__init__(
        "mcmini forward", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    global transitionId
    iterations = int(args) if args.isdigit() else 1
    if gdb.selected_inferior().num == 1:
      print("GDB is in scheduler, not target process:" +
            "  Can't go to next transition\n")
    if not self.breakpoint_for_next_transition:
      gdb.execute("break mc_shared_cv_wait_for_scheduler")
      self.breakpoint_for_next_transition = int(gdb.parse_and_eval("$bpnum"))
    else:
      gdb.execute("enable " + str(self.breakpoint_for_next_transition))
    for i in range(iterations):
      gdb.execute("continue")
    gdb.execute("disable " + str(self.breakpoint_for_next_transition))
    transitionId += 1
    print_user_frames_in_stack()
    print_mcmini_stats()
forwardCmd()

class finishTraceCmd(gdb.Command):
  """Execute until next trace; Accepts optional <count> arg"""
  breakpoint_for_next_transition = None
  def __init__(self):
    super(finishTraceCmd, self).__init__(
        "mcmini finishTrace", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    global transitionId
    if gdb.selected_inferior().num == 1:
      print("GDB is in scheduler process, not target:\n" +
            "  Try 'mcmini nextTrace' to go to next trace\n")
      return
    gdb.execute("disable " + str(gdb.parse_and_eval("$bpnum_exit")))
    gdb.execute("continue")
    gdb.execute("inferior 1")  # inferior 1 is scheduler process
    if gdb.selected_frame().name() == "waitpid":
      # Wait for zombie child, or we hit GDB bug.
      gdb.execute("finish")
    gdb.execute("enable " + str(gdb.parse_and_eval("$bpnum_exit")))
    print_mcmini_stats()
    transitionId = 0
finishTraceCmd()

class nextTraceCmd(gdb.Command):
  """Execute to next trace; Accepts optional <count> arg"""
  ###FIXME: Delete this, when McMini is stable.
  ### breakpoint_for_next_trace = None
  def __init__(self):
    super(nextTraceCmd, self).__init__(
        "mcmini nextTrace", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    iterations = int(args) if args.isdigit() else 1
    if gdb.selected_inferior().num != 1:
      # FIXME:  We should automatically call 'mcmini finishTrace' for user.
      print("GDB is in target process, not scheduler:\n" +
            "  Execute 'mcmini finishTrace' to return to scheduler\n")
      return
    ###FIXME: Delete this, when McMini is stable.
    ### if not self.breakpoint_for_next_trace:
    ###   gdb.execute("break mc_shared_cv_wait_for_scheduler")
    ###   self.breakpoint_for_next_trace = int(gdb.parse_and_eval("$bpnum"))
    ### else:
    ###   gdb.execute("enable " + str(self.breakpoint_for_next_trace))
    if iterations > 1:
      for i in range(iterations - 1):
        gdb.execute("tbreak mc_shared_cv_wait_for_scheduler")
        gdb.execute("continue")
        gdb.execute("continue")
        finishTraceCmd().invoke("",False)
    # Now continue until in child process.
    gdb.execute("tbreak mc_shared_cv_wait_for_scheduler")
    gdb.execute("continue")
    gdb.execute("continue")
    # We should now be in the next child process.
    ###FIXME: Delete this, when McMini is stable.
    ### gdb.execute("disable " + str(self.breakpoint_for_next_trace))
    print_mcmini_stats()
nextTraceCmd()

class gotoTraceCmd(gdb.Command):
  """gotoTrace <traceId>: Execute until reaching trace <traceId>    *** (NOT YET WORKING) ***"""
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
    if gdb.selected_inferior().num != 1:
      # FIXME:  We should automatically call 'mcmini finishTrace' for user.
      print("GDB is in target process, not scheduler:\n" +
            "  Execute 'mcmini finishTrace' to return to scheduler\n")
      return
    gdb.execute("set follow-fork-mode parent")
    gdb.execute("tbreak mc_search_next_dpor_branch_with_initial_thread" +
                " if traceId >= " + str(iterations))
    # FIXME:  We need to disable other breakpoint before continuing.
    #         Or else, we can go in a loop until reaching correct breakpoint.
    gdb.execute("continue")
    gdb.execute("set follow-fork-mode child")
    print_mcmini_stats()
gotoTraceCmd()

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
developerModeCmd()
