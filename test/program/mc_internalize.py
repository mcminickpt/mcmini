import sys

if len(sys.argv) < 2:
  print("USAGE:  " + sys.argv[0] + " FILE.c\n"
        "  creates mc_FILE.c; directly calling internal functions of mcmini.\n")
  sys.exit(1)

file = open(sys.argv[1])
if not file:
  print(sys.argv[0] + ": file \"" + sys.argv[1] +"\" not found")

program = file.readlines()
main_line = next((i for i in range(len(program))
                  if program[i].startswith("int main(")), None)
if not main_line:
  print(sys.argv[0] + ": file " + sys.argv[1] +":  'main' routine not found")
program.insert(main_line+2, "    mc_init();\n")

incl_line = next((i for i in range(len(program))
                  if program[i].startswith("#include")), None)
new_incl_line = next((i for i in range(incl_line, len(program)-incl_line)
                      if not program[i].startswith("#include")), None)
program.insert(new_incl_line,'#include "mcmini/mcmini.h"\n' +
                             '#include "mcmini/mcmini_wrappers.h"\n')

program = ''.join(program)

transitions = ["pthread_create", "pthread_join", "pthread_mutex_lock",
               "pthread_mutex_unlock", "pthread_mutex_init",
               "sem_init", "sem_wait", "sem_post",
               "pthread_barrier_init", "pthread_barrier_wait",
               "pthread_cond_init", "pthread_cond_wait", "pthread_cond_signal"]

for tran in transitions:
  program = program.replace(tran, "mc_" + tran)

output = open("mc_" + sys.argv[1], 'w')
output.write(program)
output.close()
