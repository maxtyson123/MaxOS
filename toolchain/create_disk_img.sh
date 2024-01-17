#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

# If the disk image already exists, delete it
if [ -f ../MaxOS.img ]; then
   msg "Image already setup"
   exit 0
fi

#Create a 2GB image
qemu-img create ../MaxOS.img 2G  || fail "Could not create image"

#Partion the image
msg "Partioning image"
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

#Try and unmount the old mount points
sudo umount /mnt/MaxOS_img_1 || warn "Couldn't unmount old mount point"
sudo umount /mnt/MaxOS_img_2 || warn "Couldn't unmount old mount point"

#Close the loop devices and attach the image to a loop device
msg "Attaching image to loop device"
sudo losetup -D
sudo losetup --partscan /dev/loop0 ../MaxOS.img  || fail "Could not mount image to loop device"

## IMAGE 1
msg "Creating filesystem for partition 1"

#Create a FAT32 Filesystem
sudo mkfs.vfat -F 32 /dev/loop0p1 || fail "Could not create filesystem"

#Create a directory for the mount point and mount the image to the mount point
sudo mkdir -p /mnt/MaxOS_img_1 || fail "Could not create mount point"
sudo mount -o loop /dev/loop0p1 /mnt/MaxOS_img_1  || fail "Could not mount image to mount point"

## IMAGE 2
msg "Creating filesystem for partition 2"

#Create a FAT32 Filesystem (partion 2)
sudo mkfs.vfat -F 32 /dev/loop0p2 || fail "Could not create filesystem"

#Create a directory for the mount point
sudo mkdir -p /mnt/MaxOS_img_2  || fail "Could not create mount point"
sudo mount -o loop /dev/loop0p2 /mnt/MaxOS_img_2  || fail "Could not mount image to mount point"

#Install grub to the image
sudo grub-install --root-directory=/mnt/MaxOS_img_1 --no-floppy --modules="normal part_msdos ext2" /dev/loop0 || fail "Could not install grub"