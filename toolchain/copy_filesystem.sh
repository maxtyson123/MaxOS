#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

msg "Copying boot files to image"


# Bootscript maps 8MB of kernel memory so ensure that the elf file is less than 8MB
KERNEL_SIZE=$($STAT_EXC -c %s "$SCRIPTDIR/../filesystem/boot/MaxOSk64")
if [ "$KERNEL_SIZE" -gt 8000000 ]; then
  fail "Error: Kernel size is greater than 8MB. Please reduce the kernel size. Or increase the size of the boot script. Kernel size: ${KERNEL_SIZE} bytes"
fi

# Copy the boot directory
sudo cp -r $SCRIPTDIR/../filesystem/boot    "$MOUNT_DIR/MaxOS_img_1"
sudo mv "$MOUNT_DIR/mnt/MaxOS_img_1/boot/grub.cfg" "$MOUNT_DIR/mnt/MaxOS_img_1/boot/grub/grub.cfg"

#Copy filesystem
msg "Copying filesystem to image"
sudo rm -rf "$MOUNT_DIR/MaxOS_img_1/os"    && sudo cp -r $SCRIPTDIR/../filesystem/os    "$MOUNT_DIR/MaxOS_img_1"
sudo rm -rf "$MOUNT_DIR/MaxOS_img_1/user"  && sudo cp -r $SCRIPTDIR/../filesystem/user    "$MOUNT_DIR/MaxOS_img_1"

# Sync filesystem
msg "Syncing filesystem"
sudo sync