import sys
import os
import subprocess as sp

try:
    pids = sp.check_output(["pgrep", "DMTCP"], text=True).strip().split("\n")
except subprocess.CalledProcessError:
    # No processes found
    print("No processes matching 'foo' were found.")
    exit(1)

# Loop through each PID and open a new GNOME terminal with gdb attached
for pid in pids:
    sp.check_output([
        "gnome-terminal", '--tab', f'--title=\'McMini Child {pid}\'',
        "--",
        "bash", "-c", f"gdb -x gdbinit-attach-dmtcp-child -p {pid}"
    ])

print(f"Opened GNOME terminals for PIDs: {', '.join(pids)}")
