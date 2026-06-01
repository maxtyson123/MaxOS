#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

# TODO: Scalability for partition size and amount of partitions
# TODO: Better loop device handling
# TODO: too much if nesting

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

# Create the images
create_fs_part() {

  local part="$1"
  local number="$2"
  msg "${part} - ${number}"

  msg "Creating filesystem for partition $number"
  sudo mkdir -p "$MOUNT_DIR/MaxOS_img_$number" || fail "Could not create mount point"

  if [ "$FILESYSTEM_TYPE" = "FAT" ]; then
    if [ "$IS_MACOS" -eq 1 ]; then
      sudo diskutil unmount "$part" || warn "Couldn't unmount $part before formatting"
      sudo mount -t msdos "$part" "$MOUNT_DIR/MaxOS_img_$number" || fail "Could not mount partition $number"
    else
      sudo mkfs.vfat -F 32 "$part" || fail "Could not create FAT32 filesystem"
      sudo mount "$part" "$MOUNT_DIR/MaxOS_img_$number" || fail "Could not mount image to mount point"
    fi
  else

    # Format the image
    MKFS="mkfs.ext2"
    if [ "$IS_MACOS" -eq 1 ]; then
      MKFS="/opt/homebrew/opt/e2fsprogs/sbin/$MKFS"
    fi
    sudo "$MKFS" "$part" || fail "Could not create EXT2 filesystem"

    if [ "$IS_MACOS" -eq 1 ]; then

      # Ensure tooling is setup
      if ! command -v fuse-ext2 >/dev/null 2>&1; then
         pushd "$SCRIPTDIR/external" > /dev/null
        ./ext2fuse.sh
        popd > /dev/null      
      fi
      sudo fuse-ext2 "/dev/r${part#/dev/}" "$MOUNT_DIR/MaxOS_img_$number" -o rw+,allow_other || fail "Could not mount image to mount point"
    else
      sudo mount "$part" "$MOUNT_DIR/MaxOS_img_$number" || fail "Could not mount image to mount point"
    fi
  fi
}
create_fs_part "$part1" "1"
create_fs_part "$part2" "2"

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
