#!/bin/bash
SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

msg "QEMU GDB Runner Started"

while true; do

  msg "Waiting for GDB to start"
  while true; do
      GDB_PID=$(pgrep -fx '/usr/bin/gdb.*')
      if  [[ -n "$GDB_PID" ]]; then
        msg "GDB Started with PID $GDB_PID"
        tmux send-keys -t 0 "make install gdb" C-m
        break
      fi
      sleep 0.2
  done

  msg "Waiting for debug session to end"
  while kill -0 "$GDB_PID" 2>/dev/null; do
    sleep 0.2
  done
  taskkill.exe /IM "qemu-system-x86_64.exe" /F

done

