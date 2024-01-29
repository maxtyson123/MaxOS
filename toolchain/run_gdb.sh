#!/bin/bash
SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

IP=localhost
IN_WSL=0
if command -v wslpath >/dev/null; then
  msg "WSL detected."

  # Get the ip from ipconfig.exe
  LOCAL_IP=${LOCAL_IP:-`ipconfig.exe | grep -im1 'IPv4 Address' | cut -d ':' -f2`}

  echo "WSL IP is: ${LOCAL_IP}"
  IP=${LOCAL_IP}

  # Strip the carriage return
  IP=${IP%$'\r'}
fi


# Make the GDB .init file
cat > ~/.gdbinit <<EOF

# Load the OS
symbol-file ../MaxOS.sym
target remote $IP:1234

# Pretty printing
set pagination off
set print pretty on
set print address on
set print symbol-filename on

# Split  screen
layout split

# Dont ask to confirm quit
define hook-quit
    set confirm off
end

#### Current debugging ###
b kernelMain
continue
EOF

# Run GDB
gdb

# Delete the GDB .init file
#rm ~/.gdbinit