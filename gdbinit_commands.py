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
##   (gdb) python print(dir(gdbObject))
##   Python stack on error is off by defautt:
##     In Python:  gdb.execute("set python print-stack full")

## EXAMPLE USAGE of Python API breakpoints:
##   bkptMain = gdb.Breakpoint("main")
##   bkptMain.silent = True
##   bkptMain.enabled = False
##   bkptMain.thread = <Thread>
##   gdb.breakpoints()

## FIXME (but now obsolete):
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
## 4. I'm not a fan of using 'operator' widely for the same reason that
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
##  We could set thread name of the McMini scheduler thread for inferior 1:
##    prctl (PR_SET_NAME, "MCMINI_INTERNAL", 0, 0, 0)
##  to distinguish it from the user threads.

# ===========================================================
# We now do the setup.  If  '-p <traceSeq>' found, add '-p 0'.
# Call subprocess with './mcmini' first, to get a full traceSeq.

import os, subprocess, time
# "MCMINI_ARGS" is set in mcmini-gdb, mcmini-python.py
assert "MCMINI_ARGS" in os.environ
mcmini_args = os.getenv("MCMINI_ARGS")
del os.environ["MCMINI_ARGS"]

def insert_extra(args, extra):
  args = args.split()
  last_flag_idx = max([ idx for (idx, word) in enumerate(args)
                            if word.startswith('-') or word.startswith("'-") ],
                      default = -1)
  last_word = args[last_flag_idx].replace("'", "")
  if last_word in ["--max-depth-per-thread", "--print-at-trace", "-m", "-p"]:
    last_flag_idx += 1
  args = args[:last_flag_idx+1] + extra.split() + args[last_flag_idx+1:]
  return ' '.join(args)

if "-p0" not in mcmini_args.split() and "'-p' '0'" not in mcmini_args:
  # If "-p0" not in the mcmini arguments, then get the trace sequence first.
  # We will then add "-p 0 -p <traceSeq>" to the command line before giving
  #   control to gdb.

  # 'mcmini' is the exec-file for GDB
  exec_file =  gdb.execute("info files", to_string=True).split('\n')[0]
  exec_file = exec_file.split('"')[1]
  # FIXME:  If it needs '-m', it won't print.  Check if max-limit reaached,
  #         and then suggest to user to add '-m'.
  cmd = exec_file + " -v -q " + mcmini_args
  print("** Generating trace sequence for:\n     " + cmd)
  print("     (This may take a while ...)")
  mcmini_output = subprocess.run(cmd, shell=True, capture_output=True,
                                 timeout=300)
  error_output = mcmini_output.stderr.decode('utf-8')
  if "Missing target_executable" in error_output:
    print([line for line in error_output.split('\n')
                if "Missing target_executable" in line][0], file=sys.stderr)
    sys.exit(1)
  mcmini_output = mcmini_output.stdout.decode('utf-8').split('\n')
  pending_indexes = [idx for idx, line in enumerate(mcmini_output)
                         if "THREAD PENDING OPERATIONS" in line]
  # '-v' will insert many "THREAD PENDING" msg's; Use last one for '-f'
  if pending_indexes:
     trace_seq = mcmini_output[pending_indexes[-1]-2]
  else:
    trace_indexes = [idx for idx, line in enumerate(mcmini_output)
                         if line.startswith("TraceId ")]
    if trace_indexes:
      print("** Found %d traces" % len(trace_indexes))
      print("     (Choosing the last trace)")
      trace_seq = mcmini_output[trace_indexes[-1]].split(':')[1].strip()
    else:
      error_indexes = [idx for idx, line in enumerate(mcmini_output)
                           if "McMini ran a trace" in line]
      if error_indexes:
        print("\n" + '\n'.join(mcmini_output[error_indexes[0]:]))
      else:
        print("******** mcmini-gdb: Internal error:"
              " can't compute trace sequence")
      gdb.execute("quit")
  extra_args = " -p0 -p'" + trace_seq + "' "

  mcmini_args = mcmini_args.split()
  # Strip "'" if it surrounds an arg with no spaces:
  for i in range(len(mcmini_args)):
    if len(mcmini_args[i].strip("'"))+2 == len(mcmini_args[i]):
      mcmini_args[i] = mcmini_args[i].strip("'")
  # Now, rejoin the edited words in mcmini_args
  def delete_one(elt, args):
    if len([1 for arg in args if arg == elt]) > 1:
      args[args.index(elt)] = ""
  # mcmini-gdb must not use '-f' cmd; Else upon last transition, scheduler exits
  mcmini_args = [ arg for arg in mcmini_args if "-f" != arg ]
  delete_one("-q", mcmini_args)
  delete_one("-v", mcmini_args)
  # Remove any old prefixes: '-p0', '-p 0', '-p 0,0, ...', etc.
  tmp = [idx for idx, line in enumerate(mcmini_args)
                 if line.startswith("-p")]
  for idx in tmp:
    if mcmini_args[idx] in ["-p", "--print-at-trace"]:
      mcmini_args[idx+1] = "" # "-p" takes an argumnet; Remove next word
    mcmini_args[idx] = ""

  mcmini_args = ' '.join([arg for arg in mcmini_args if arg != ""])
  mcmini_args = insert_extra(mcmini_args, extra_args)
  print("** Running: " + exec_file + "-gdb " + mcmini_args)
  print("** Note:  In order to replay this trace,\n" +
        "          it is faster to directly run the above command line.\n")
  time.sleep(2) # Allow user some time to read the above message
  gdb.execute("set args " + mcmini_args)
  ### FIXME:  We must now instantiate the new argumnets before '(gdb) run'.

def is_traceSeq(arg):
  numbers = arg.strip().strip("'" + '"').replace(",", " ")
  if "'" in numbers:
    numbers = numbers.split("'")[0]
  if '"' in numbers:
    numbers = numbers.split('"')[0]
  numbers = numbers.split()
  is_trace = len(numbers) > 1 and len([ num for num in numbers
                                           if num.isnumeric() ]) == len(numbers)
  return (is_trace, len(numbers))
def trace_seq_len(mcminiArgs):
  if "--print-at-trace" in mcminiArgs:
    args = mcminiArgs.split("--print-at-trace")[1:]
  elif "-p" in mcminiArgs and [a for a in mcminiArgs.split("-p")[1:]
                                 if is_traceSeq(a)[0]]:
    args = mcminiArgs.split("-p")[1:]
  else:
    return 0
  args = [arg for arg in args if is_traceSeq(arg)[0]]
  return is_traceSeq(args[0])[1]


def is_tui_active():
  return "The TUI is not active." not in gdb.execute("info win", to_string=True)

# This version of gdb.execute() checks if the inferior is no longer running.
def mcmini_execute(command, to_string=False):
  # Other possible tests:  is_valid() and len(inferior.threads()) != 0
  try:
    inferior = gdb.inferiors()[-1]
    if inferior.pid == 0:
      raise gdb.error
    return gdb.execute(command, to_string=to_string)
  except gdb.error:
    print("\n*** The program is not being run anymore.")
    print("*** The last transition id was: *** "+str(transitionId)+" ***")
    print("*** mcmini-gdb is now exiting.\n")
    if is_tui_active():
      gdb.execute("refresh") # This makes TUI active and refreshes
    gdb.execute("set confirm off")
    gdb.execute("quit")

def total_num_frames():
  frame = gdb.newest_frame()
  while frame.older(): # Get oldest frame
    frame = frame.older()
  return frame.level() + 1

def select_user_frame():
  gdb.invalidate_cached_frames()  # Is this useful?
  frame = gdb.newest_frame()
  while frame.older(): # Get oldest frame
    frame = frame.older()
    if frame.name() == "mc_exit_main_thread":
      frame.select()
      return
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
    # For forcing TUI redisplay.
    # FIXME:  Don't do this if the TUI already knows about our frame.
    #         For example, 'mcmini where/print' doesn't need to change it.
    #         GDB commands: up; down;  fix the display, but print to output
    #                       and leave current line at top.
    gdb.execute("frame " + str( gdb.selected_frame().level() ))
    ## GDB BUG: 'list <LINE>' should help TUI to center display; but this fails.
    # if gdb.execute("frame " + str( gdb.selected_frame().level() )):
    #   gdb.execute("list " + str( gdb.selected_frame().find_sal().line ))
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
  output = gdb.execute("bt " + str(- (total_num_frames() -
                                      gdb.selected_frame().level())),
                       to_string=True)
  print(output)
def find_call_frame(name):
  frame = gdb.newest_frame()
  while frame:
    if frame.name() == name:
      return frame
    frame = frame.older()
  return None

# ===========================================================
# Set up breakpoint utilities

## This has the nice advantage of skipping over any user breakpoints.
def continue_until(function, thread_id=None):
  ## Could use "temporary" optional argument, but gdb 7.6 doesn'support it.
  ## bkpt = gdb.Breakpoint("main", gdb.BP_BREAKPOINT, gdb.WP_WRITE.
  ##              False, True) # Optional args:  internal=True, temporary=True
  bkpt = gdb.Breakpoint(function, gdb.BP_BREAKPOINT, gdb.WP_WRITE, True)
  bkpt.silent = True
  bkpt_exit = gdb.Breakpoint("_exit", gdb.BP_BREAKPOINT, gdb.WP_WRITE, True)
  bkpt_exit.silent = True
  bkpt_terminate_trace = gdb.Breakpoint("mc_terminate_trace",
                                        gdb.BP_BREAKPOINT, gdb.WP_WRITE, True)
  bkpt_terminate_trace.silent = True
  if thread_id:
    bkpt.thread = thread_id
  while bkpt.hit_count == 0 and bkpt_exit.hit_count == 0 and \
        bkpt_terminate_trace.hit_count == 0:
    mcmini_execute("continue")
  if bkpt_exit.hit_count > 0:
    gdb.execute("inferior 1")
    finish()
    gdb.execute("inferior 1")
    finish()
  if bkpt_terminate_trace.hit_count > 0:
    gdb.execute("inferior " + str(gdb.inferiors()[-1].num))
  bkpt.delete()
  bkpt_exit.delete()
  bkpt_terminate_trace.delete()

def finish():
  # GDB "finish" would write to screen even when trying to make it silent.
  if not gdb.selected_thread() and (gdb.selected_inferior().num == 1
       or gdb.inferiors()[0].num == 1 and gdb.inferiors()[0].pid == 0):
    breakpoint()
  try:
    bkpt = gdb.FinishBreakpoint(internal=True)
  except:
    breakpoint()
  bkpt.silent = True
  gdb.execute("continue")
  if not gdb.selected_thread():
    gdb.execute("inferior 1")  # User thread must have exited.

# NOTE: gdb.Breakpoint.stop() can be defined to do anything arbitrary when
#                       reaching the breakpoint, such as print a message.

# ===========================================================
# Redirect output:  gdb-msg -> /dev/null; McMini -> mcprintf_redirect()

dup_stdout = -1 # uninitialized
output = "REDIRECT UNINTIALIZED"
user_inferior = -1

def redirect_prolog(inferior=1):
  # NOTE: This only partially workis in TUI; it sends to curses, not stdout. :-(
  #        So we capture McMini output and re-print it later to TUI.
  # FIXME: We could do tui-disable; update; tui-enable to get around it. ??
  # We now replace original stdout/stderr by /dev/null
  # Turn pagination off; GDB junk should not go to paginated stream.
  thr_user = gdb.selected_thread()
  thr_scheduler =  gdb.inferiors()[0].threads()[0]
  ## FIXME:  If thr.switch() is now silent, do we still need stdout->/dev/null ?
  ##         (And do we still need mcprintf_redirect, for TUI pagination?)
  cur_pagination = gdb.parameter("pagination")
  gdb.set_parameter("pagination", "off")
  # GDB 'inferior XXX' normally tries to print filename, and errors and
  #   and sends to stderr Stop trying to print filename.  This prevents that.
  cur_frame_info = gdb.execute("show print frame-info", to_string=True)
  cur_frame_info = cur_frame_info.split('"')[1]
  gdb.execute("set print frame-info location")
  dup_stdout = os.dup(1)
  os.close(1)
  cur_stdout = os.open('/dev/null', os.O_WRONLY)
  assert cur_stdout == 1
  # GDB messages will now go to /dev/null
  thr_scheduler.switch()
  gdb.execute("call mcprintf_redirect()")
  cur_stdout = os.open('/dev/null', os.O_WRONLY)
  if inferior == 1: # inferior 1 is scheduler process
    thr_scheduler.switch()
  else:
    gdb.execute("inferior " + str(inferior)) 
  os.dup2(dup_stdout, 1)
  return (dup_stdout, thr_user, inferior, cur_pagination, cur_frame_info)

def redirect_epilog(context, print_hack = False):
  (dup_stdout, thr_user, inferior, cur_pagination, cur_frame_info) =context
  ## FIXME:  If thr.switch() is now silent, do we still need stdout->/dev/null ?
  ##         (And do we still need mcprintf_redirect, for TUI pagination?)
  os.close(1)
  cur_stdout = os.open('/dev/null', os.O_WRONLY)
  thr_scheduler =  gdb.inferiors()[0].threads()[0]
  if inferior == 1:
    thr_scheduler.switch()  # inferior 1 is scheduler process
  else:
    gdb.execute("inferior " + str(inferior))
  gdb.execute("call mcprintf_stop_redirect()")
  if not thr_user.is_valid():
    thr_user = gdb.selected_thread()
  output = gdb.parse_and_eval("mcprintf_redirect_output").string()
  # Under TUI, this seems to go to curses (or stderr?), not stdout:
  gdb.execute("set print frame-info " + cur_frame_info)
  thr_user.switch()
  select_user_frame()
  if is_tui_active():
    gdb.execute("frame " + str(gdb.selected_frame().level()))
  # Return to original stdout/stderr
  # FIXME: When is_tui_active(), some things don't go to stdout
  os.close(1)
  os.dup2(dup_stdout, 1)
  os.close(dup_stdout)
  # It's now safe to print
  gdb.flush()
  gdb.set_parameter("pagination", "on" if cur_pagination else "off")
  # We need this hack because GDB TUI doesn't erase part of first line.
  if is_tui_active() and print_hack:
    print(" === ")
    output = "*** " + output
  print(output)
  ## OR:  gdb.execute('printf "' + output.replace("\n", "\\n") + '"')
  ## OR:  gdb.write(output)
  gdb.flush()
  if is_tui_active():
    # GDB BUG: Doing: mcmini forward 6; mcmini printTransitions; ^Xa; up-arrow
    #      then sets TUI src window to "No source available".
    #      If we type this below manually, it refreshes, but not under Python.
    gdb.execute("frame " + str( gdb.selected_frame().level() ))
    gdb.execute("refresh")

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
* CONSIDER USING:  ctrl-Xa ('ctrl-X' and 'a'), ctrl-Xo, 'winheight src -5'     *
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
    has_exited = False
    if gdb.newest_frame().name() in ["__GI__exit", "_exit"]:
      has_exited = True
    if not has_exited:
      context = redirect_prolog(inferior=1)
      gdb.execute("call programState->printTransitionStack()")
      assert gdb.selected_inferior().num == 1
      gdb.execute("call programState->printNextTransitions()")
      redirect_epilog(context, print_hack=True)
    else:
      print("Process has exited")
printTransitionsCmd()

class printPendingTransitionsCmd(gdb.Command):
  """Prints the next (pending) transition for each thread"""
  def __init__(self):
    super(printPendingTransitionsCmd, self).__init__(
        "mcmini printPendingTransitions", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    context = redirect_prolog(inferior=1)
    gdb.execute("call programState->printNextTransitions()")
    redirect_epilog(context)
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
  """Execute until next transition; Accepts optional arg: <count>; or: end """
  def __init__(self):
    super(forwardCmd, self).__init__(
        "mcmini forward", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    global transitionId
    args = args.split()
    iterations = int(args[0]) if args and args[0].isdigit() else 1
    if "end" in args:  # if 'mcmini forward end'
      iterations = trace_seq_len(mcmini_args) - transitionId
    if iterations > 1:
      mcmini_execute("mcmini forward " + str(iterations-1) + " quiet")
    if gdb.selected_inferior().num == 1 and gdb.inferiors()[-1].num > 0:
      gdb.execute("inferior " + str(gdb.inferiors()[-1].num))

    if iterations == 0:
      print_current_frame_verbose()
      return
    # Are we at the end of the trace?
    thr_user = gdb.selected_thread()
    gdb.inferiors()[0].threads()[0].switch() # inferior 1
    if gdb.newest_frame().name().startswith("mc_terminate_trace") == 1:
      print("\n*** Target process exited;" +
            " (Try: 'mcmini printTransitions' and/or 'mcmini back')\n")
      thr_user.switch()
      return
    thr_user.switch()
    # GDB on top of Red Hat uses debuginfo files.  This will suppress
    # the warning.  However, this GDB command fails on Debian-based distros,
    # and anyway, no warning about missing debug files issued.
    try:
      gdb.execute("set build-id-verbose 0")
    except:
      pass
    continue_until("mc_shared_sem_wait_for_scheduler_done")
    # FIXME: There can be many aliases for "_exit".  We should use address.
    if gdb.selected_inferior().pid == 0 or \
       gdb.newest_frame().name() == "__GI__exit":
      if gdb.selected_inferior().pid != 0:
        gdb.execute("inferior " + str(gdb.inferiors()[-1].num))
        gdb.execute("set unwindonsignal on")
      print("\n*** McMini target process has exited." +
            "  Suggestion: 'mcmini printTransitions'")
      return
    if gdb.newest_frame().name() == "mc_shared_sem_wait_for_scheduler_done":
      finish()
    # Three possibilities:  __GI_exit; *_scheduler__done; and mc_terminate_trace
    # In the last case, process it as normal, and allow user to inspect/bo gack
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
  """Go back <count> transitions, by re-executing; default count=1"""
  def __init__(self):
    super(backCmd, self).__init__(
        "mcmini back", gdb.COMMAND_USER
    )
  def invoke(self, args, from_tty):
    global transitionId
    args = args.split()
    count = int(args[0]) if args and args[0].isdigit() else 1
    if gdb.selected_inferior().num == 1 and gdb.inferiors()[-1].num > 0:
      gdb.execute("inferior " + str(gdb.inferiors()[-1].num))
    if gdb.selected_inferior().num == 1:
      print("No target available.  Did it exit?")
      return
    iterationsForward = transitionId - count
    if iterationsForward < 0:
      print("ERROR: Trying to go back past beginning:" +
            " transitionId=%d; count=%d" % (transitionId, count))
      return
    context = redirect_prolog()
    transitionId = 0
    inferior_num = gdb.selected_inferior().num
    if gdb.selected_inferior().num != 1: gdb.execute("inferior 1")
    gdb.execute("set mc_reset = 1")
    # After finishing, this kills the current child, and interrupts
    # with the sigchld_handler_scheduler.
    while not gdb.newest_frame().name().startswith("mc_restore_initial_trace"):
      continue_until("mc_restore_initial_trace")
    assert "mc_restore_initial_trace" in gdb.newest_frame().name()
    # Kill the old child inferior
    if gdb.selected_inferior().num != 1: gdb.execute("inferior 1")
    while "wait" not in gdb.newest_frame().name() and \
          "fork" not in gdb.newest_frame().name():
      if gdb.selected_inferior().num != 1: gdb.execute("inferior 1")
      # if gdb.newest_frame().name() == "arch_fork":
      #   breakpoint()
      finish()
    # Three cases:
    #  1. The newest frame is "wait"
    #  2. Newest frame is sigchildhandler (due to "mc_terminate); and "wait"
    #  3. The newest frame is "arch_fork" (called by " __libc_fork")
    #     [ Why does it sometimes skip over 'wait' and go to
    #       arch_fork?  The old child seems to exist but then soon disappear. ]
    if not ( "wait" in gdb.newest_frame().name() or \
             "wait" in gdb.newest_frame().older().name() or \
             "fork" in gdb.newest_frame().name() ):
      breakpoint()
    assert "wait" in gdb.newest_frame().name() or \
           "wait" in gdb.newest_frame().older().name() or \
           "fork" in gdb.newest_frame().name()
    if "fork" in gdb.newest_frame().name():
      finish()
      gdb.execute("inferior " + str(gdb.inferiors()[-1].num))
      continue_until("main")
      assert gdb.newest_frame().name() == "main"
      redirect_epilog(context)
      # We're now at the beginning of the trace (user: "main").
      # After this, stap at mc_shared_sem_wait_for_scheduler_done for transition.
      gdb.execute("mcmini forward " + str(iterationsForward))
      return
    finish() # Finish it for one more time.
    assert len(gdb.inferiors()) == 1
    # And create the new child inferior
    continue_until("mc_fork_next_trace_at_current_state")
    if not "mc_fork_next_trace_at_current_state" in gdb.newest_frame().name():
      breakpoint()
    gdb.execute("catch syscall fork")
    if not "fork" in gdb.newest_frame().name():
      breakpoint()
    assert "fork" in gdb.newest_frame().name()
    gdb.execute("inferior 1")
    finish()
    assert len(gdb.inferiors()) > 1
    gdb.execute("inferior " + str(gdb.inferiors()[-1].num))
    # We're now at the beginning of the trace (user: "mcmini_main") constructor.
    continue_until("main")
    assert gdb.newest_frame().name() == "main"
    redirect_epilog(context)
    # We're now at the beginning of the trace (user: "main").
    # After this, stap at mc_shared_sem_wait_for_scheduler_done for transition.
    gdb.execute("mcmini forward " + str(iterationsForward))
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
whereCmd()

# NOT USED:
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

# NOT USED:
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

# NOT USED:
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
    # user_inferior = gdb.selected_inferior().num
    # gdb.execute("inferior 1") # Set inferior to scheduler
    # scheduler_call_frame_fnc = "mc_shared_sem_wait_for_thread"
    # gdb.execute("break " + scheduler_call_frame_fnc)
    # This next command forces a GDB-internal bug in gdb-12.0
    # gdb.FinishBreakpoint().__init__(find_call_frame_fnc(scheduler_call_frame_fnc))
    # gdb.execute("inferior " + str(user_inferior))
    gdb.execute("inferior 1")
    gdb.execute("set print address on")
    gdb.execute("set detach-on-fork on")
    gdb.execute("set follow-fork-mode parent")
    for inferior in gdb.inferiors():
      if inferior.num > 1:
        gdb.execute("detach inferior " + str(int(inferior.num)))
    print(developerHelp)
developerModeCmd()
