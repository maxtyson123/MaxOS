#!/bin/bash

#TODO: rsync Mac, unmount/remount mac

SCRIPTDIR=$(dirname "$BASH_SOURCE")
MOUNT_DIR="$(cd "$SCRIPTDIR/.." && pwd)/filesystem"
source $SCRIPTDIR/MaxOS.sh

# Parse the args
REVERSE=0
while [ "$#" -gt "0" ]; do
  case "$1" in
    --reverse)
      REVERSE=1
      shift 1
      ;;
    *)
      warn "Error: Unknown argument $1"
      ;;
  esac
done

#TODO MAKE SURE MOUNTED

# Locations
BOOT_DIR="$SCRIPTDIR/../filesystem/0/boot/"
INIT_DIR="$SCRIPTDIR/../filesystem/0/boot/initrd/"

# Bootscript maps 8MB of kernel memory so ensure that the elf file is less than 8MB (todo: I think I changed this)
KERNEL_SIZE=$($STAT_EXC -c %s "$BOOT_DIR/MaxOSk64")
if [ "$KERNEL_SIZE" -gt 8000000 ]; then
  fail "Error: Kernel size is greater than 8MB. Please reduce the kernel size. Or increase the size of the boot script. Kernel size: ${KERNEL_SIZE} bytes"
fi

DESTINATION="$MOUNT_DIR/0"

# Create the inital ram disk
INIT_FILE="$(cd "$SCRIPTDIR/../filesystem/0/boot" && pwd)/maxos.initrd"
rm -f "$INIT_FILE"
ls $INIT_DATA >/dev/null 2>&1 || fail "Cant find init files: $INIT_DIR/*"
( cd "$INIT_DIR" && tar -cf "$INIT_FILE" --no-xattrs * )

: "${USE_ISO:=0}"
# Produce an ISO? default to no
if [ "$USE_ISO" -eq 1 ]; then

  # Cant pull changes from the iso
  if [ "$REVERSE" -ne 1 ]; then
    exit 0
  fi

  DESTINATION="$SCRIPTDIR/../iso"
  if [ ! -d "$DESTINATION" ]; then
    mkdir -p "$DESTINATION"
  fi
fi

# Create the iso
if [ "$USE_ISO" -eq 1 ]; then
  msg "Creating ISO"
  i686-elf-grub-mkrescue --modules="part_msdos fat normal" --output="$SCRIPTDIR/../MaxOS.iso" $DESTINATION || fail "Failed to create rescue ISO"
  rm -rf $DESTINATION 2>/dev/null || sudo rm -rf $DESTINATION
fi
