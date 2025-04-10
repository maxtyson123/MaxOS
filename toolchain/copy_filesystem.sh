#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

msg "Copying boot files to image"


# Bootscript maps 8MB of kernel memory so ensure that the elf file is less than 8MB
KERNEL_SIZE=$(stat -c %s "$SCRIPTDIR/../filesystem/boot/MaxOSk64")
if [ "$KERNEL_SIZE" -gt 8000000 ]; then
  fail "Error: Kernel size is greater than 8MB. Please reduce the kernel size. Or increase the size of the boot script. Kernel size: ${KERNEL_SIZE} bytes"
fi

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