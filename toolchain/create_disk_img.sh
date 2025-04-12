#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

#TODO: Scalibitlity for partition size and amount of partitions

# If the disk image already exists no need to setup
if [ -f ../MaxOS.img ]; then
    msg "Image already exists"
    exit 0
fi

# Remove the disk on failure TODO: Also unmount the image
ON_FAIL="rm -f ../MaxOS.img"

#Create a 2GB image
qemu-img create ../MaxOS.img 2G  || fail "Could not create image"

#Partion & Mount the image
dev=""
msg "Partitioning image"
if [ "$IS_MACOS" -eq 1 ]; then
   dev_arr=($(hdiutil attach -nomount ../MaxOS.img))
   dev=${dev_arr[0]}
   sudo diskutil partitionDisk $dev MBRFormat "MS-DOS FAT32" "BOOT" 1G "MS-DOS FAT32" "DATA" R
else
  fdisk ../MaxOS.img -u=cylinders << EOF
  o
  n
  p
  1
  1
  130
  n
  p
  2
  131
  243
  a
  1
  w
EOF

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
if [ -f ../MaxOS.img ]; then
  msg "MacOS: Image mounted already"
  part1="${dev}s1"
  part2="${dev}s2"
  sudo diskutil unmount /Volumes/BOOT
  sudo diskutil unmount /Volumes/DATA

else
  msg "Attaching image to loop device"
  sudo losetup -D
  sudo losetup --partscan /dev/loop0 ../MaxOS.img  || fail "Could not mount image to loop device"
fi

## IMAGE 1
msg "Creating filesystem for partition 1"
sudo mkdir -p "$MOUNT_DIR/MaxOS_img_1" || fail "Could not create mount point"
if [ "$IS_MACOS" -eq 1 ]; then
  sudo newfs_msdos -F 32 -v BOOT "$part1" || fail "Could not create filesystem on partition 1"
  sudo mount -t msdos "$part1" "$MOUNT_DIR/MaxOS_img_1" || fail "Could not mount partition 1"
else
  sudo mkfs.vfat -F 32 /dev/loop0p1 || fail "Could not create filesystem"
  sudo mount -o loop /dev/loop0p1 "$MOUNT_DIR/MaxOS_img_1"  || fail "Could not mount image to mount point"
fi


## IMAGE 2
msg "Creating filesystem for partition 2"
sudo mkdir -p "$MOUNT_DIR/MaxOS_img_2"  || fail "Could not create mount point"
if [ "$IS_MACOS" -eq 1 ]; then
  sudo diskutil unmount "$part1" || warn "Couldn't unmount $part1 before formatting"
  sudo newfs_msdos -F 32 -v BOOT "$part2" || fail "Could not create filesystem on partition 1"
  sudo mount -t msdos "$part2" "$MOUNT_DIR/MaxOS_img_2" || fail "Could not mount partition 1"
else
   sudo diskutil unmount "$part2" || warn "Couldn't unmount $part2 before formatting"
  sudo mkfs.vfat -F 32 /dev/loop0p2 || fail "Could not create filesystem"
  sudo mount -o loop /dev/loop0p2 "$MOUNT_DIR/MaxOS_img_2"  || fail "Could not mount image to mount point"
fi

#Install grub to the image
if [ "$IS_MACOS" -eq 1 ]; then
  warn "Grub not supported on MacOS, please install grub manually"
else
  sudo grub-install --root-directory="$MOUNT_DIR/MaxOS_img_1" --no-floppy --modules="normal part_msdos ext2" $dev || fail "Could not install grub"
fi
