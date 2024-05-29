#!/usr/bin/env python

import os
import sys
import subprocess

os.environ["MCMIN_ANNOTATE"] = "1"

newargs = [ arg if ' ' not in arg else "'"+arg+"'" for arg in sys.argv ][1:]

# Capture "export MCMINI_ROOT=..." from mcmini-gdb shell script
if '/' in sys.argv[-1]:
  mcmini_root = '/'.join(sys.argv[0].split('/')[:-1])
else:
  mcmini_root = os.getcwd()
# Could verify mcmini_root with an alternative code:
#   path = [ line for line in open(mcmini_root + "/" + mcmini_gdb).readine() if line.startswith("export MCMINI_ROOT=") ]
#   mcmini_root = (path[0]+"-END$").split('=')[1].replace("/mcmini-gdb-END$", "")

cmd = "gdb -x $MCMINI_ROOT/gdbinit -x $MCMINI_ROOT/gdbinit_annotate " + \
      "--args $MCMINI_ROOT/mcmini" + " -p 0 " + ' '.join(newargs)
cmd = cmd.replace("$MCMINI_ROOT", mcmini_root).encode('utf-8')

output = subprocess.run(cmd, shell=True, capture_output=True, timeout=300).stdout
output = output.decode('utf-8')
if 'THREAD PENDING OPERATIONS' not in output.split('\n') \
   and "*** transition: " not in output:
  error_output = subprocess.run(mcmini_root + "/mcmini" + newargs, shell=True,
                      capture_output=True, timeout=300).stderr.decode('utf-8')
  if "Missing target_executable" in error_output:
    print("\n")
    print([line for line in error_output.split('\n')
                if "Missing target_executable" in line][0], file=sys.stderr)
  else:
    print( "\n*** mcmini-annotate.py: Invalid arguments." +
           " Please check args and target binary.", file=sys.stderr)
  sys.exit(1)

pending = ""
if "THREAD PENDING OPERATIONS" in output:
  pending = output.split('THREAD PENDING OPERATIONS')[1].split('\n')
  pending[0] = 'THREAD PENDING OPERATIONS'
  pending = pending[:pending.index("END")] + [""]

end_trace_idx = output.rindex("> Thr")
end_trace_idx = end_trace_idx + output[end_trace_idx:].index('\n') + 1
output = output[:end_trace_idx].split('\n')

output1 = [ line for line in output if line.startswith("*** transition: ") and
                                      not line.startswith("thread 0: starts") ]
output2 = [ "  " + line for line in output if line.startswith("> ") ]
output2 = [ line.replace(": ", ":\n    ... ") for line in output2 ]
output2 = [ line.replace(":\n    ... ", ": ... ", 1) for line in output2 ]
output2 = [ line.replace("\n    ... ", "\n           ... ") for line in output2 ]

output2 = [ line.replace("thread_await_scheduler()",
                         "thread_await_scheduler() [thread exits]")
            for line in output2 ]
output2 = [ line.replace("mc_thread_routine_wrapper(...)",
                         "mc_thread_routine_wrapper(...) [thread starts]")
            for line in output2 ]

new_output = []
for pair in list(zip(output1, output2)):
  new_output += pair

print('\n' + '\n'.join(new_output) + '\n')
if pending:
  print("========================================")
  print('\n'.join(pending))
