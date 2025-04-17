#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

msg "Copying boot files to image"

# Bootscript maps 8MB of kernel memory so ensure that the elf file is less than 8MB
KERNEL_SIZE=$($STAT_EXC -c %s "$SCRIPTDIR/../filesystem/boot/MaxOSk64")
if [ "$KERNEL_SIZE" -gt 8000000 ]; then
  fail "Error: Kernel size is greater than 8MB. Please reduce the kernel size. Or increase the size of the boot script. Kernel size: ${KERNEL_SIZE} bytes"
fi

DESTINATION="$MOUNT_DIR/MaxOS_img_1"

# MacOS uses an ISO for now
if [ "$USE_ISO" -eq 1 ]; then
  DESTINATION="$SCRIPTDIR/../iso"
  if [ ! -d "$DESTINATION" ]; then
    mkdir -p "$DESTINATION"
  fi
fi

#Copy filesystem
msg "Copying filesystem to image"
sudo rm -rf "$DESTINATION/boot"    && sudo cp -r $SCRIPTDIR/../filesystem/boot   $DESTINATION
sudo rm -rf "$DESTINATION/os"    && sudo cp -r $SCRIPTDIR/../filesystem/os   $DESTINATION
sudo rm -rf "$DESTINATION/user"  && sudo cp -r $SCRIPTDIR/../filesystem/user $DESTINATION

# Sync filesystem
msg "Syncing filesystem"
sudo sync

# Create the iso
if [ "$USE_ISO" -eq 1 ]; then
  msg "Creating ISO"
  i686-elf-grub-mkrescue --modules="part_msdos fat normal" --output="$SCRIPTDIR/../MaxOS.iso" $DESTINATION || fail "Failed to create rescue ISO"
  sudo rm -rf $DESTINATION
fi