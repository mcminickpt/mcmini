# Nice tutorial:
# https://interrupt.memfault.com/blog/automate-debugging-with-gdb-python-api

## NOTE:  Inside gdb, you can test Python commands.  For example:
##   (gdb) python gdb.execute("bt")
##   (gdb) python print(gdb.parse_and_eval("foo(x)")) # for foo(), x in target
## INTERACTIVE DEBUGGING:
##   (gdb) python-interactive
##   Insert:  'import pdb; pdb.set_trace()' or 'breakpoint()'
##   # Invoke with count of 3 and 'True' (default: fromTtty; for all commands)
##   (gdb) python nextTransitionCmd().invoke("3",True)

## EXAMPLE USAGE of Python API breakpoints:
##   bkptMain = gdb.Breakpoint("main")
##   bkptMain.silent = True
##   bkptMain.enabled = False
##   bkptMain.thread = <Thread>
##   gdb.breakpoints()

## FIXME:
## mcmini nextTrace <count> fails, when using optional <count> argument.

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
## 4. In WSL, the arguments of the target program are getting lost. WHY?
##    As a result, producer_consumer doesn't recognize its '--quiet' flag.
## 5. I'm not a fan of using 'operator' widely for the same reason that
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
# NEW for use with:  ./mcmini -p <traceSeq>

def is_tui_active():
  return "The TUI is not active." not in gdb.execute("info win", to_string=True)

# This version of gdb.execute() checks if the inferior is no longer running.
def mcmini_execute(command, to_string=False):
  # Other possible tests:  is_valid() and len(inferior.threads()) != 0
  try:
    ### This code should also work.  But instead, it creates recursion error.
    ###   even for a simple "mcmini execute".  Why?
    ## inferior = [inf for inf in gdb.inferiors() if inf.num == 2][0]
    ## if inferior.pid != 0:
    ##   while (1): print("HI\n"); True
    ##   raise gdb.error
    return gdb.execute(command, to_string=to_string)
  except gdb.error:
    print("\n*** The program is not being run anymore.")
    print("*** The last transition id was: *** "+str(transitionId)+" ***")
    print("*** mcmini-gdb is now exiting.\n")
    if is_tui_active():
      gdb.execute("refresh") # This makes TUI active and refreshes
    gdb.execute("set confirm off")
    gdb.execute("quit")

def total_frames():
  frame = gdb.newest_frame()
  while frame.older(): # Get oldest frame
    frame = frame.older()
  return frame.level() + 1

def select_user_frame():
  gdb.invalidate_cached_frames()
  frame = gdb.newest_frame()
  while frame.older(): # Get oldest frame
    frame = frame.older()
  while True:
    # Search from oldest to newest for a call frame whose _next_ (newer) frame
    #   would be a McMini call frame.
    if frame.name() and not frame.newer(): # This must be user main, at start.
      frame.select()
      break
    if frame.find_sal().symtab and not frame.newer().find_sal().symtab:
      frame.select() # Maybe newer is McMini frame not compiled with '-g'.
      break
    if not frame.find_sal().symtab and frame.newer().find_sal().symtab:
      frame.select() # This could be start_thread(), with McMini frame as newer
      break
    if not frame.find_sal().symtab and not frame.newer().find_sal().symtab:
      frame = frame.newer()
      continue # Probably this is:  clone3; start_thread; ...
    assert frame.find_sal().symtab and frame.newer().find_sal().symtab
    if ( frame.name() and
         frame.newer().find_sal().symtab and  # always true of McMini frames
         # If this is "starts" transition of new thread., ignore _newer_ frames
         ( frame.newer().name() == "mc_thread_routine_wrapper"
           and
           frame.newer().newer().find_sal().symtab
           and
           frame.newer().newer().name() ==
           "thread_await_scheduler_for_thread_start_transition") or
         ( frame.newer().name() != "mc_thread_routine_wrapper" and
           ( "src/transitions/" in frame.newer().find_sal().symtab.filename or
             "src/mcmini_private.cpp" in frame.newer().find_sal().symtab.filename))):
      frame.select() # This is a user call frame or else the start of a new thread.
      break
    frame = frame.newer()
  frame.select()
  if is_tui_active():
    # We've selected the frame, but the GDB 'frame' cmd will now tell the TUI.
    # For forcing TUI redisplay, alternatives to the GDB 'frame' cmd might be
    # "tui disable; tui enable", or GDB "update" cmd, or GDB "down; up".
    gdb.execute("frame " + str( gdb.selected_frame().level() ))
    gdb.execute("refresh")

# ===========================================================
# Print statistics

transitionId = 0
def print_mcmini_stats():
  global transitionId
  print("*** transition: " + str(transitionId) + "; "
        "thread: " + str(gdb.selected_inferior().num) + "." +
                     str(gdb.selected_thread().num) +
                 " (thread " + str(gdb.selected_thread().num) +
                 " of inferior " + str(gdb.selected_inferior().num) + ")"
        "\n")

def print_user_frames_in_stack():
  if gdb.selected_inferior().num == 1:
    print("Internal error: print_user_frames_in_stack called " +
                           "outside user program\n")
  select_user_frame()
  # gdb.execute would print newline, but not carriage return:
  printout = gdb.execute("bt " + str(- (total_frames() -
                                        gdb.selected_frame().level())),
                         to_string=True)
  print(printout)
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
  ## We would have preferred a temporary breakpoint.
  ## gdb 7.6 doesn't seem to implement "temporary" optional argument.
  ## Optional arguments:  internal=False, temporary=True
  ## bkpt = gdb.Breakpoint("main", gdb.BP_BREAKPOINT, gdb.WP_WRITE.
  ##                       False, True)
  # Optional argument:  internal=True
  bkpt = gdb.Breakpoint(function, gdb.BP_BREAKPOINT, gdb.WP_WRITE, True)
  bkpt.silent = True
  if thread_id:
    bkpt.thread = thread_id
  while bkpt.hit_count == 0:
    mcmini_execute("continue")
  bkpt.delete()

def continue_beyond(function):
  # Last two arguments:  internal: False; temporary: True
  ## This variant fails with "bad syntax" in some GDB/Python versions.
  # bkpt = gdb.Breakpoint("main", gdb.BP_BREAKPOINT, gdb.WP_WRITE. False, True)
  bkpt = gdb.Breakpoint("main", gdb.BP_BREAKPOINT, gdb.WP_WRITE)
  bkpt.silent = True
  while bkpt.isValid:
    mcmini_execute("continue")
  # Continue until pre-existing breakpoint
  mcmini_execute("continue")

def finish():
  # GDB "finish" would write to screen even when trying to make it silent.
  bkpt = gdb.FinishBreakpoint(internal=True)
  bkpt.silent = True
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
* Use GDB commands 'up' and 'down' to view other call frames.                  *
* By default, cursor keys browse the source code, not the command history.     *
* Type 'focus cmd'/'focus src' (or ctrl-Xo) to change the focus for cmd & src. *
*   Note: You cannot scroll back to see previous command output.               *
*                                                                              *
* Note that for certain technical reasons in the implementation of McMini,     *
* certain thread functions (e.g., sem_wait, pthread_cond_wait)                 *
* will appear to be executed twice when calling 'mcmini forward'.              *
*                                                                              *
* For details of 'mcmini' commands, type 'help user-defined' and the online    *
* McMini manual.                                                               *
*                                                                              *
* CONSIDER USING ctrl-Xa ('ctrl-X' and 'a') TO TOGGLE SOURCE DISPLAY ON OR OFF.*
********************************************************************************
"""
)

class helpCmd(gdb.Command):
  """Prints help for getting started in McMini"""
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
   gdb.execute("call programState->printTransitionStack()")
   gdb.execute("call programState->printNextTransitions()")
   gdb.execute("inferior " + str(current_inferior))
printTransitionsCmd()

class printPendingTransitionsCmd(gdb.Command):
  """Prints the next (pending) transition for each thread"""
  def __init__(self):
    super(printPendingTransitionsCmd, self).__init__(
        "mcmini printPendingTransitions", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
   current_inferior = gdb.selected_inferior().num
   gdb.execute("inferior 1")  # inferior 1 is scheduler process
   gdb.execute("call programState->printNextTransitions()")
   gdb.execute("inferior " + str(current_inferior))
printPendingTransitionsCmd()

import re
def extract_fnc_call(fnc_name, source_line):
  file, line = source_line.split(':')
  source_line2 = file + ":" + str(int(line)+1)
  list_cmd = "list " + source_line + ", " + source_line2
  if "output styling is enabled." in gdb.execute("show style enabled",
                                                 to_string=True):
    gdb.execute("set style enabled off")
    extract = gdb.execute(list_cmd, to_string=True)
    gdb.execute("set style enabled on")
  else:
    extract = gdb.execute(list_cmd, to_string=True)
  # reg = r'(\x9B|\x1B\[)[0-?]*[ -\/]*[@-~]' # remove ANSCO terminal chars
  # extract = re.sub(reg, '', extract)
  if fnc_name not in extract:
    return fnc_name + "(...)"
  idx = extract.index(fnc_name)
  count = 0
  idx2 = idx
  for i in range(idx + len(fnc_name), len(extract)):
    idx2 = i+1
    if extract[i] == '(': count += 1
    if extract[i] == ')': count -= 1
    if count == 0: break
  return extract[idx : idx2].replace('\n', "")

def print_current_frame_verbose():
  select_user_frame()
  frame = gdb.selected_frame()
  print_mcmini_stats()
  if frame.find_sal().symtab:
    source_line = frame.find_sal().symtab.filename.split('/')[-1] + ":" + \
                str(frame.find_sal().line)
    if frame.newer(): # This must be user main, at start.
      source_function_call = extract_fnc_call(frame.newer().name(), source_line)
    else: # This must be user main, at start.
      source_function_call = "** START **"
    print("> Thr " + str(gdb.selected_thread().num - 1) + ": Inside " +
          frame.name() + "() [" +
          frame.find_sal().symtab.filename.split('/')[-1] + ":" +
          str(frame.find_sal().line) + "]: " + source_function_call)
  else:
    print("> Thr " + str(gdb.selected_thread().num - 1) + ": Inside " +
          frame.name() + "() [" + "??" + "]: " + "??")

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
      mcmini_execute("mcmini forward " + str(iterations-1) + " quiet")
    # else iterations == 1
    if gdb.selected_inferior().num == 1:
      print("GDB is in scheduler, not target process:" +
            "  Can't go to next transition\n")
      return
    if iterations == 0:
      print_current_frame_verbose()
      return
    # GDB on top of Red Hat uses debuginfo files.  This will suppress
    # the warning.  However, this GDB command fails on Debian-based distros,
    # and anyway, no warning about missing debug files issued.
    try:
      gdb.execute("set build-id-verbose 0")
    except:
      pass
    continue_until("mc_shared_sem_wait_for_scheduler_done")
    finish()
    transitionId += 1
    if "quiet" not in args:
      select_user_frame()
      frame = gdb.selected_frame()
      if frame.find_sal().symtab:
        print_current_frame_verbose()
      else:
        print_mcmini_stats()
        print("> Thr " + str(gdb.selected_thread().num - 1) + ": Inside " +
              frame.name() + "() [" +
              "No source file info; Was it compiled with '-g'?]")
      select_user_frame()
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
    transitionId = 0
    inferior_num = gdb.selected_inferior().num
    if gdb.selected_inferior().num != 1: gdb.execute("inferior 1")
    gdb.execute("set mc_reset = 1")
    # After finishing, this kills the current child, and interrupts
    # with the sigchld_handler_scheduler.
    continue_until("mc_restore_initial_trace")
    finish() # finish() is interrupted by signal:  sigchld_handler_scheduler
    # Continue to kill off the old inferior and create a new one.
    if gdb.selected_inferior().num != 1: gdb.execute("inferior 1")
    while gdb.newest_frame().name() != "mc_search_dpor_branch_with_thread":
      if gdb.selected_inferior().num != 1: gdb.execute("inferior 1")
      finish()
      if gdb.selected_inferior().num != 1: gdb.execute("inferior 1")
    gdb.execute("inferior " + str(gdb.inferiors()[-1].num))
    continue_until("mc_shared_sem_wait_for_scheduler_done")
    # We're now at the beginning of the trace (user: "mcmini_main") constructor.
    continue_until("main")
    # We're now at the beginning of the trace (user: "main").
    # After this, stap at mc_shared_sem_wait_for_scheduler_done for transition.
    gdb.execute("mcmini forward " + str(iterationsForward))
    print("*** Still need to implement 'mcmini back <count>'")
backCmd()

class whereCmd(gdb.Command):
  """Execute where, while hiding McMini internal call frames"""
  def __init__(self):
    super(whereCmd, self).__init__(
        "mcmini where", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    print("STACK FOR THREAD: " + str(gdb.selected_thread().num))
    print_user_frames_in_stack()
    print_mcmini_stats()
    select_user_frame()
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
    # gdb.execute("enable " + str(gdb.parse_and_eval("$bpnum_exit")))
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
# finishTraceCmd()  # Not working with '-p <traceSeq>'

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
        continue_until("mc_shared_sem_wait_for_scheduler")
        gdb.execute("mcmini finishTrace")
    # Now continue until in child process.
    continue_until("mc_shared_sem_wait_for_scheduler")
    # We should now be in the next child process.
    if "quiet" not in args:
      print_mcmini_stats()
# nextTraceCmd()  # Not working with '-p <traceSeq>'

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
    if gdb.selected_inferior().num != 1:
      gdb.execute("mcmini finishTrace quiet")
    # We should now have only the parent (inferior 1).
    gdb.execute("set detach-on-fork on")
    # Optional argument:  internal=False ## FIXME: Change to True when ready.
    # FIXME:  If fifth arg here, it says only 4 args.  But 5 args work above.
    bkpt = gdb.Breakpoint("mc_search_dpor_branch_with_thread" +
                            "(unsigned long)",
                          gdb.BP_BREAKPOINT, gdb.WP_WRITE, True)
    bkpt.silent = True
    while int(gdb.parse_and_eval("traceId")) < iterations:
      gdb.execute("continue")
    bkpt.delete()
    gdb.execute("set detach-on-fork off")
    gdb.execute("mcmini nextTrace quiet")
    print_mcmini_stats()
# gotoTraceCmd()  # Not working with '-p <traceSeq>'

developerHelp = ("""\
Executes:
  inferior 1
  detach inferior [DETACHES ALL OTHER INFERIORS]
  set detach-on-fork on
  set follow-fork-mode parent
  [ Reverse these if you want to again debug the target process. ]
Useful GDB commands:
  info inferiors
  inferior 1
    [ Inferior 1 is the scheduler process. ]
  info threads
  thread 2.1
    [ Thread 2.1 is thread 1 of inferior 2 ]
  info breakpoints
  where
""")

class developerModeCmd(gdb.Command):
  """Permanently switch GDB to developer environment.  For developers only."""
  def __init__(self):
    super(developerModeCmd, self).__init__(
        "mcmini developerMode", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    print("Breakpoint added at next visible operation in scheduler process.")
    gdb.execute("break mc_run_thread_to_next_visible_operation(unsigned long)")
    ### These commented commands will go away, when it's clear it's not needed.
    # current_inferior = gdb.selected_inferior().num
    # gdb.execute("inferior 1") # Set inferior to scheduler
    # scheduler_call_frame_fnc = "mc_shared_sem_wait_for_thread"
    # gdb.execute("break " + scheduler_call_frame_fnc)
    # This next command forces a GDB-internal bug in gdb-12.0
    # gdb.FinishBreakpoint().__init__(find_call_frame_fnc(scheduler_call_frame_fnc))
    # gdb.execute("inferior " + str(current_inferior))
    gdb.execute("inferior 1")
    gdb.execute("set print address on")
    gdb.execute("set detach-on-fork on")
    gdb.execute("set follow-fork-mode parent")
    for inferior in gdb.inferiors():
      if inferior.num > 1:
        gdb.execute("detach inferior " + str(int(inferior.num)))
    print(developerHelp)
developerModeCmd()
