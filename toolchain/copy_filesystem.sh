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

# Produce an ISO? default to no
: "${USE_ISO:=0}"
if [ "$USE_ISO" -eq 1 ]; then
  DESTINATION="$SCRIPTDIR/../iso"
  if [ ! -d "$DESTINATION" ]; then
    mkdir -p "$DESTINATION"
  fi
fi

# Linux: setup loop device to write to
dev=""
IMAGE="../MaxOS.img"
if [ "$IS_MACOS" -ne 1 ]; then
  msg "Mounting img"
  dev=$(sudo losetup --find --show --partscan "$IMAGE")
  sudo mount "$dev"p1 "$MOUNT_DIR/MaxOS_img_1"
  sudo mount "$dev"p2 "$MOUNT_DIR/MaxOS_img_2"
fi

# Syncing local filesystem
msg "Copying filesystem to image"
sudo rsync --no-owner --no-group -a --delete -c "$SCRIPTDIR/../filesystem/"  "$MOUNT_DIR/MaxOS_img_1/"
#TODO: rsync Mac, unmount/remount mac

# Create the iso
if [ "$USE_ISO" -eq 1 ]; then
  msg "Creating ISO"
  i686-elf-grub-mkrescue --modules="part_msdos fat normal" --output="$SCRIPTDIR/../MaxOS.iso" $DESTINATION || fail "Failed to create rescue ISO"
  sudo rm -rf $DESTINATION
fi

# Linux: clean up the loop device
if [ "$IS_MACOS" -ne 1 ]; then
  sudo umount "$dev"p1 "$dev"p2
  sudo losetup -d "$dev"
fi