# Configure GDB in a reasonable way
set breakpoint pending on
set pagination off
# GDB will track both parent and child
set detach-on-fork off
set print pretty
## set print address off
###############################################
# Consider this for more silent operation, but add back for developer mode
## set print frame-info off
## set print inferior-events off # This appears with new trace; Maybe set it off for 'mcmini back'
## set print symbol-loading off
# In McMini, parent sends SIGUSR1 to child on exit.
handle SIGUSR1 nostop noprint pass
handle SIGUSR2 nostop noprint pass
# Allow the other inferior to continue to execute if not at breakpoint
set schedule-multiple on
## Optional for additional modes for threads/inferiors:
## SEE: https://stackoverflow.com/questions/27140941/preventing-debugging-session-from-pausing-after-each-inferior-exits
# set non-stop on
# set target-async on

# Suppress "Missing separate debuginfos" message:
## This is now done in 'mcmini forward'
## set build-id-verbose 0

# source [$MCMINI_ROOT/]gdbinit_commands.py"
python import os, sys
python DIR = ""
python if os.environ.get("MCMINI_ROOT"): DIR = os.environ["MCMINI_ROOT"]+"/"
python if DIR: gdb.execute("dir " + DIR)
python if os.path.isfile("NO-GDB-G3"): \
  print("\n*** Not compiled with -g3.  Please call:\n" + \
        "***                             make clean && make -j9 debug"); \
  sys.exit(1)
python print("source " + DIR + "gdbinit_commands.py")
python gdb.execute("source " + DIR + "gdbinit_commands.py")

tbreak execvp
run

python if (not gdb.selected_inferior().threads()): sys.exit(1)

tbreak 'mcmini_main()'
continue

# Continue through 'fork' and into 'main' for target (child) process
tbreak main
continue

# Print Python-based GDB commands:
help user-defined
echo \n\ \ *** Type 'mcmini help' for usage. ***\n
echo     \ \ (Do 'set print address off' for less verbose output.)\n\n
