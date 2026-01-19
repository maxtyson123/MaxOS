#!/bin/bash

#TODO: rsync Mac, unmount/remount mac

SCRIPTDIR=$(dirname "$BASH_SOURCE")
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

if [ "$REVERSE" -ne 1 ]; then
  msg "Copying boot files to image"
else
  msg "Pulling changes made during run"
fi

# Locations
BOOT_DIR="$SCRIPTDIR/../filesystem/boot/"
INIT_DIR="$SCRIPTDIR/../filesystem/initrd/"

# Bootscript maps 8MB of kernel memory so ensure that the elf file is less than 8MB (todo: I think I changed this)
KERNEL_SIZE=$($STAT_EXC -c %s "$BOOT_DIR/MaxOSk64")
if [ "$KERNEL_SIZE" -gt 8000000 ]; then
  fail "Error: Kernel size is greater than 8MB. Please reduce the kernel size. Or increase the size of the boot script. Kernel size: ${KERNEL_SIZE} bytes"
fi

DESTINATION="$MOUNT_DIR/MaxOS_img_1"

# Create the inital ram disk
INIT_FILE="$(cd "$SCRIPTDIR/../filesystem/boot" && pwd)/maxos.initrd"
rm -f "$INIT_FILE"
ls $INIT_DATA >/dev/null 2>&1 || fail "Cant find init elfs: $INIT_DIR/*.elf"
( cd "$INIT_DIR" && tar -cf "$INIT_FILE" --no-xattrs --disable-copyfile *.elf )

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
if [ "$REVERSE" -ne 1 ]; then
  msg "Copying filesystem to image"
  sudo rsync --exclude='._*' --no-xattrs --no-o --no-g -a --delete -c "$SCRIPTDIR/../filesystem/"  "$MOUNT_DIR/MaxOS_img_1/"
else
  msg "Copying changes on image to local filesystem"
  sudo rsync --exclude='._*' --no-xattrs --itemize-changes --chown=$(id -un):$(id -gn) -a --delete -c "$MOUNT_DIR/MaxOS_img_1/" "$SCRIPTDIR/../filesystem/"
fi

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