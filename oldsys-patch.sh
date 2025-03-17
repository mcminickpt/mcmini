#!/bin/sh

sed --in-place \
  -e 's^capture_output=True,^stdout=subprocess.PIPE, stderr=subprocess.PIPE,^'\
  gdbinit_commands.py

sed --in-place -e 's^\([a-z._\(\)]*\)\.level()^level(\1)^' gdbinit_commands.py

sed --in-place -e '/show style enabled/N; s/^\( *\)[^ ].*show style enabled.*$/\1if False:/' gdbinit_commands.py

sed --in-place -e 's/^\( *\)gdb.set_parameter.*$/\1pass/' gdbinit_commands.py

sed --in-place -e 's/^\( *\)cur_pagination = gdb.parameter.*$/\1pass/' gdbinit_commands.py

sed --in-place -e 's/^\( *\).*print frame-info.*$/\1pass/' gdbinit_commands.py

sed --in-place -e 's/^\( *\)cur_frame_info = .*$/\1cur_frame_info = ""/' gdbinit_commands.py

sed --in-place -e 's/^\( *\)context = redirect_prolog(inferior=1).*$/\1gdb.execute("inferior 1")/' gdbinit_commands.py

sed --in-place -e 's/^\( *\)context = redirect_prolog().*$/\1pass/' gdbinit_commands.py

sed --in-place -e 's/^\(  *\)redirect_epilog(.*$/\1pass/' gdbinit_commands.py

cat <<'EOF' | cat - >> gdbinit_commands.py

def level(frame):
  frame2 = gdb.newest_frame()
  count = 0
  while frame != frame2 and frame2.older():
    frame2 = frame2.older()
    count += 1
  return count
EOF
