#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

# TODO: Scalability for partition size and amount of partitions
# TODO: Better loop device handling

IMAGE="../MaxOS.img"

# If the disk image already exists no need to setup
if [ -f "$IMAGE" ]; then
    msg "Image already exists"
    exit 0
fi

# Remove the disk on failure TODO: Also unmount the image
ON_FAIL="rm -f $IMAGE"

#Create a 2GB image
qemu-img create "$IMAGE" 2G  || fail "Could not create image"

#Partion & Mount the image
dev=""
msg "Partitioning image"
if [ "$IS_MACOS" -eq 1 ]; then
   dev_arr=($(hdiutil attach -nomount ../MaxOS.img))
   dev=${dev_arr[0]}
   sudo diskutil partitionDisk $dev MBRFormat "MS-DOS FAT32" "BOOT" 1G "MS-DOS FAT32" "DATA" R
else

    if [ "$FILESYSTEM_TYPE" = "FAT" ]; then
      TYPE_CODE="b"  # FAT32
    else
      TYPE_CODE="83" # EXT2
    fi

  fdisk "$IMAGE" -u=cylinders << EOF
  o
  n
  p
  1
  1
  130
  t
  $TYPE_CODE
  a
  1
  n
  p
  2
  131
  243
  t
  2
  $TYPE_CODE
  w
EOF
  dev=$(sudo losetup --find --show --partscan "$IMAGE")
fi

#Try and unmount the old mount points
if mount | grep -q "$MOUNT_DIR/MaxOS_img_1"; then
    sudo umount "$MOUNT_DIR/MaxOS_img_1" || warn "Couldn't unmount old mount point"
fi
if mount | grep -q "$MOUNT_DIR/MaxOS_img_2"; then
    sudo umount "$MOUNT_DIR/MaxOS_img_2 "|| warn "Couldn't unmount old mount point"
fi

# Get the partions
part1=""
part2=""
if [  "$IS_MACOS" -eq 1 ]; then
  part1="${dev}s1"
  part2="${dev}s2"
  sudo diskutil unmount /Volumes/BOOT
  sudo diskutil unmount /Volumes/DATA

else
  msg "Attaching image to loop device"
  part1="${dev}p1"
  part2="${dev}p2"
fi
msg "${part1}"
msg "${part2}"

## IMAGE 1
msg "Creating filesystem for partition 1"
sudo mkdir -p "$MOUNT_DIR/MaxOS_img_1" || fail "Could not create mount point"
if [ "$FILESYSTEM_TYPE" = "FAT" ]; then
  if [ "$IS_MACOS" -eq 1 ]; then
    sudo diskutil unmount "$part1" || warn "Couldn't unmount $part1 before formatting"
    sudo mount -t msdos "$part1" "$MOUNT_DIR/MaxOS_img_1" || fail "Could not mount partition 1"
  else
    sudo mkfs.vfat -F 32 "$part1" || fail "Could not create FAT32 filesystem"
    sudo mount "$part1" "$MOUNT_DIR/MaxOS_img_1" || fail "Could not mount image to mount point"
  fi
else
  if [ "$IS_MACOS" -eq 1 ]; then
    fail "EXT2 is not supported on macOS by default"
  else
    sudo mkfs.ext2 "$part1" || fail "Could not create EXT2 filesystem"
    sudo mount "$part1" "$MOUNT_DIR/MaxOS_img_1" || fail "Could not mount image to mount point"
  fi
fi

## IMAGE 2
msg "Creating filesystem for partition 2"
sudo mkdir -p "$MOUNT_DIR/MaxOS_img_2" || fail "Could not create mount point"

if [ "$FILESYSTEM_TYPE" = "FAT" ]; then
  if [ "$IS_MACOS" -eq 1 ]; then
    sudo diskutil unmount "$part2" || warn "Couldn't unmount $part2 before formatting"
    sudo mount -t msdos "$part2" "$MOUNT_DIR/MaxOS_img_2" || fail "Could not mount partition 2"
  else
    sudo mkfs.vfat -F 32 "$part2" || fail "Could not create FAT32 filesystem"
    sudo mount "$part2" "$MOUNT_DIR/MaxOS_img_2" || fail "Could not mount image to mount point"
  fi
else
  if [ "$IS_MACOS" -eq 1 ]; then
    fail "EXT2 is not supported on macOS by default"
  else
    sudo mkfs.ext2 "$part2" || fail "Could not create EXT2 filesystem"
    sudo mount "$part2" "$MOUNT_DIR/MaxOS_img_2" || fail "Could not mount image to mount point"
  fi
fi

# Sync
msg "Syncing filesystem"
sync
sudo sync

# Define grub modules
GRUB_MODULES="normal part_msdos biosdisk echo multiboot2"
if [ "$FILESYSTEM_TYPE" = "FAT" ]; then
  GRUB_MODULES="$GRUB_MODULES fat"
fi

if [ "$FILESYSTEM_TYPE" = "EXT2" ]; then
  GRUB_MODULES="$GRUB_MODULES ext2"
fi

#Install grub to the image
if [ "$IS_MACOS" -eq 1 ]; then
  msg "Installing GRUB manually on macOS"

    CORE_IMG=core.img
    GRUB_PREFIX="(hd0,msdos1)/boot/grub"
    GRUB_LOCAL_DIR="/opt/homebrew/Cellar/i686-elf-grub/2.12/lib/i686-elf/grub/i386-pc"

    # Build the GRUB core image
    i686-elf-grub-mkimage -O i386-pc -o $CORE_IMG -p "$GRUB_PREFIX" $GRUB_MODULES  || fail "Could not create GRUB core image"

    # Write GRUB bootloader (boot.img + core.img) to disk image
    BOOT_IMG="$GRUB_LOCAL_DIR/boot.img"
    sudo dd if=$BOOT_IMG of=../MaxOS.img bs=446 count=1 conv=notrunc || fail "Failed to write boot.img to $dev"
    sudo dd if=$CORE_IMG of=../MaxOS.img bs=512 seek=1 conv=notrunc || fail "Failed to write core.img to $dev"

    # Copy GRUB modules to the image
    sudo mkdir -p "$SCRIPTDIR/../filesystem/boot/grub/i386-pc/" || fail "Could not create grub directory"
    sudo cp -r "$GRUB_LOCAL_DIR/"*.mod "$SCRIPTDIR/../filesystem/boot/grub/i386-pc/" || fail "Could not copy grub modules to image"

    rm -f $CORE_IMG

    msg "GRUB manually installed to disk image"
    sync
    sudo sync
else
  msg "Installing GRUB to disk image: $dev"
  sudo grub-install --root-directory="$MOUNT_DIR/MaxOS_img_1" --no-floppy --modules="$GRUB_MODULES" "$dev" || fail "Could not install grub"
  sudo umount "${part1}" "${part2}"
  sudo losetup -d "$dev"
fi