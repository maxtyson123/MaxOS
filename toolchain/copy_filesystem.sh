#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

msg "Copying boot files to image"

# Copy the boot directory
sudo cp -r $SCRIPTDIR/../filesystem/boot    /mnt/MaxOS_img_1
sudo mv /mnt/MaxOS_img_1/boot/grub.cfg /mnt/MaxOS_img_1/boot/grub/grub.cfg


#Copy filesystem
msg "Copying filesystem to image"
sudo rm -rf /mnt/MaxOS_img_1/os    && sudo cp -r $SCRIPTDIR/../filesystem/os    /mnt/MaxOS_img_1
sudo rm -rf /mnt/MaxOS_img_1/user  && sudo cp -r $SCRIPTDIR/../filesystem/user    /mnt/MaxOS_img_1

# Sync filesystem
msg "Syncing filesystem"
sudo sync