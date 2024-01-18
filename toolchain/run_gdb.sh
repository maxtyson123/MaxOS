#!/bin/bash
SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

# Generate the debug symbols
objcopy --only-keep-debug $SCRIPTDIR/../filesystem/boot/MaxOSk64 ../MaxOS.sym
msg "Generated debug symbols"

# Make the GDB .init file
cat > ~/.gdbinit <<EOF

# Defaults
symbol-file ../MaxOS.sym
target remote 172.29.16.1:1234
set print pretty on

# Current debugging
b kernelMain
continue
EOF

# Run GDB
gdb

# Delete the GDB .init file
rm ~/.gdbinit