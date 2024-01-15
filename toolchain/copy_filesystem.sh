#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

msg "Copying boot files to image"

# Copy the boot directory
sudo cp -r $SCRIPTDIR/../filesystem/boot    /mnt/MaxOS_img_1
sudo mv /mnt/MaxOS_img_1/boot/grub.cfg /mnt/MaxOS_img_1/boot/grub/grub.cfg


#Copy filesystem
msg "Copying filesystem to image"
sudo rm -rf /mnt/MaxOS_img_1/bin    && sudo cp -r $SCRIPTDIR/../filesystem/bin    /mnt/MaxOS_img_1
sudo rm -rf /mnt/MaxOS_img_1/dev    && sudo cp -r $SCRIPTDIR/../filesystem/dev    /mnt/MaxOS_img_1
sudo rm -rf /mnt/MaxOS_img_1/etc    && sudo cp -r $SCRIPTDIR/../filesystem/etc    /mnt/MaxOS_img_1
sudo rm -rf /mnt/MaxOS_img_1/home   && sudo cp -r $SCRIPTDIR/../filesystem/home   /mnt/MaxOS_img_1
sudo rm -rf /mnt/MaxOS_img_1/lib    && sudo cp -r $SCRIPTDIR/../filesystem/lib    /mnt/MaxOS_img_1
sudo rm -rf /mnt/MaxOS_img_1/media  && sudo cp -r $SCRIPTDIR/../filesystem/media  /mnt/MaxOS_img_1
sudo rm -rf /mnt/MaxOS_img_1/opt    && sudo cp -r $SCRIPTDIR/../filesystem/opt    /mnt/MaxOS_img_1
sudo rm -rf /mnt/MaxOS_img_1/tmp    && sudo cp -r $SCRIPTDIR/../filesystem/tmp    /mnt/MaxOS_img_1
sudo rm -rf /mnt/MaxOS_img_1/usr    && sudo cp -r $SCRIPTDIR/../filesystem/usr    /mnt/MaxOS_img_1
sudo rm -rf /mnt/MaxOS_img_1/var    && sudo cp -r $SCRIPTDIR/../filesystem/var    /mnt/MaxOS_img_1

# Sync filesystem
msg "Syncing filesystem"
sudo sync